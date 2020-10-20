
import {
    Body,
    Path,
    Header,
    Query,
    SuccessResponse,
    Response,
    Controller,
    Get,
    Post,
    Put,
    Route,
} from "@tsoa/runtime";

import * as _ from 'lodash-es'
import * as math from 'mathjs'

import { ValidateErrorResponse, ValidateErrorStatus } from "../error_handler"
import { parseAuthToken } from "../util"
import * as ccf from "../types/ccf"

export const MINIMUM_OPINION_THRESHOLD = 10

interface ErrorResponse {
    message: string
}

interface CreatePollRequest {
    type: "string" | "number"
}

interface SubmitOpinionRequest {
    opinion: string | number
}

type SubmitOpinionsRequest = { [topic: string]: SubmitOpinionRequest }

// TODO does this even make sense? maybe during poll creation the string choices should be recorded?
// TODO rename to ChoicePoll?
interface StringPollResponse {
    type: "string"
    statistics?: {
        counts: { [ opinion: string]: number}
    }
    opinion?: string
}

interface NumericPollResponse {
    type: "number"
    statistics?: {
        mean: number
        std: number
    }
    opinion?: number
}

type GetPollResponse = StringPollResponse | NumericPollResponse

type GetPollsResponse = { [topic: string]: GetPollResponse }

// Export REST API request/response types for unit tests
export {
    CreatePollRequest, SubmitOpinionRequest, SubmitOpinionsRequest,
    GetPollResponse, StringPollResponse, NumericPollResponse 
}

// KV schema
type User = string

interface PollBase<T> {
    creator: string
    type: string
    opinions: Record<User, T>
}

interface StringPoll extends PollBase<string> {
    type: "string"
}

interface NumericPoll extends PollBase<number> {
    type: "number"
}

type Poll = StringPoll | NumericPoll

@Route("polls")
export class PollController extends Controller {

    private kvPolls = new ccf.TypedKVMap(ccf.kv.polls, ccf.string, ccf.json<Poll>())
    private kvTopics = new ccf.TypedKVMap(ccf.kv.topics, ccf.string, ccf.json<string[]>())
    private kvTopicsKey = 'all'

    @SuccessResponse(201, "Poll has been successfully created")
    @Response<ErrorResponse>(403, "Poll has not been created because a poll with the same topic exists already")
    @Response<ValidateErrorResponse>(ValidateErrorStatus, "Schema validation error")
    //@Post('{topic}') // CCF does not support url templates yet for JS apps
    @Post()
    public createPoll(
        //@Path() topic: string,
        @Query() topic: string,
        @Body() body: CreatePollRequest,
        // TODO should be handled via @Security
        @Header() authorization: string,
    ): void {
        const user = parseAuthToken(authorization)

        if (this.kvPolls.has(topic)) {
            this.setStatus(403)
            return { message: "Poll with given topic exists already" } as any
        }
        this.kvPolls.set(topic, {
            creator: user,
            type: body.type,
            opinions: {}
        })
        const topics = this._getTopics()
        topics.push(topic)
        this.kvTopics.set(this.kvTopicsKey, topics)
        this.setStatus(201)
    }

    @SuccessResponse(204, "Opinion has been successfully recorded")
    @Response<ErrorResponse>(400, "Opinion was not recorded because the opinion data type does not match the poll type")
    @Response<ErrorResponse>(404, "Opinion was not recorded because no poll with the given topic exists")
    @Response<ValidateErrorResponse>(ValidateErrorStatus, "Schema validation error")
    //@Put('{topic}')
    @Put()
    public submitOpinion(
        //@Path() topic: string,
        @Query() topic: string,
        @Body() body: SubmitOpinionRequest,
        @Header() authorization: string,
    ): void {
        const user = parseAuthToken(authorization)

        try {
            var poll = this.kvPolls.get(topic)
        } catch (e) {
            this.setStatus(404)
            return { message: "Poll does not exist" } as any
        }
        if (typeof body.opinion !== poll.type) {
            this.setStatus(400)
            return { message: "Poll has a different opinion type" } as any
        }      
        poll.opinions[user] = body.opinion
        this.kvPolls.set(topic, poll)
        this.setStatus(204)
    }

    @SuccessResponse(204, "Opinions have been successfully recorded")
    @Response<ErrorResponse>(400, "Opinions were not recorded because either an opinion data type did not match the poll type or a poll with the given topic was not found")
    @Response<ValidateErrorResponse>(ValidateErrorStatus, "Schema validation error")
    //@Put('{topic}')
    @Put('all')
    public submitOpinions(
        @Body() body: SubmitOpinionsRequest,
        @Header() authorization: string,
    ): void {
        const user = parseAuthToken(authorization)

        for (const [topic, submitOpinionRequest] of Object.entries(body)) {
            try {
                var poll = this.kvPolls.get(topic)
            } catch (e) {
                this.setStatus(400)
                return { message: `Poll with topic '${topic}' does not exist` } as any
            }
            if (typeof submitOpinionRequest.opinion !== poll.type) {
                this.setStatus(400)
                return { message: `Poll with topic '${topic}' has a different opinion type` } as any
            }      
            poll.opinions[user] = submitOpinionRequest.opinion
            this.kvPolls.set(topic, poll)
        }

        this.setStatus(204)
    }

    @SuccessResponse(200, "Poll data")
    @Response<ErrorResponse>(404, "Poll data could not be returned because no poll with the given topic exists")
    @Response<ValidateErrorResponse>(ValidateErrorStatus, "Schema validation error")
    //@Get('{topic}')
    @Get()
    public getPoll(
        //@Path() topic: string
        @Query() topic: string,
        @Header() authorization: string,
    ): GetPollResponse {
        const user = parseAuthToken(authorization)

        if (!this.kvPolls.has(topic)){
            this.setStatus(404)
            return { message: "Poll does not exist" } as any
        }

        this.setStatus(200)
        return this._getPoll(user, topic)
    }

    
    @SuccessResponse(200, "Poll data")
    @Response<ValidateErrorResponse>(ValidateErrorStatus, "Schema validation error")
    @Get('all')
    public getPolls(
        @Header() authorization: string,
    ): GetPollsResponse {
        const user = parseAuthToken(authorization)

        let polls: GetPollsResponse = {}

        for (const topic of this._getTopics()) {
            polls[topic] = this._getPoll(user, topic)
        }

        this.setStatus(200)
        return polls
    }

    _getTopics(): string[] {
        try {
            return this.kvTopics.get(this.kvTopicsKey)
        } catch (e) {
            return []
        }
    }

    _getPoll(user: string, topic: string): GetPollResponse {
        try {
            var poll = this.kvPolls.get(topic)
        } catch (e) {
            throw new Error(`Poll with topic '${topic}' does not exist`)
        }

        const opinionCountAboveThreshold = Object.keys(poll.opinions).length >= MINIMUM_OPINION_THRESHOLD

        let res: GetPollResponse
        // TODO can repetition be avoided while maintaining type checking?
        if (poll.type == "string") {
            res = {
                type: poll.type,
                opinion: poll.opinions[user]
            }
            if (opinionCountAboveThreshold) {
                const opinions = Object.values(poll.opinions)
                res.statistics = {
                    counts: _.countBy(opinions)
                }
            }
        } else if (poll.type == "number") {
            res = {
                type: poll.type,
                opinion: poll.opinions[user]
            }
            if (opinionCountAboveThreshold) {
                const opinions = Object.values(poll.opinions)
                res.statistics = {
                    mean: math.mean(opinions),
                    std: math.std(opinions)
                }
            }
        } else {
            throw new Error('unknown poll type')
        }
        return res
    }
}