import {
    Request,
    Controller,
    Post,
    Route,
  } from "@tsoa/runtime";

// Importing the browser bundle works around https://github.com/protobufjs/protobuf.js/issues/1402.
import protobuf from 'protobufjs/dist/protobuf.js'

// The OpenAPI spec is overridden in endpoints.json
// as tsoa only supports application/json.
// We use @Request and 'any' types as backdoor to get access to CCF's
// request object without applying any validation in advance.

@Route("proto")
export class ProtoController extends Controller {

  @Post()
  public wrapInProtobuf(
    @Request() request: any
  ): any {
    // Example from https://github.com/protobufjs/protobuf.js.
    let Type = protobuf.Type;
    let Field = protobuf.Field;

    let AwesomeMessage = new Type("AwesomeMessage").add(new Field("awesomeField", 1, "string"));

    let message = AwesomeMessage.create({ awesomeField: request.body.text() });
    let arr = AwesomeMessage.encode(message).finish();

    this.setHeader('content-type', 'application/x-protobuf')
    return arr;
  }
}