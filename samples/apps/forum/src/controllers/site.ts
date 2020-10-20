
import {
    Hidden,
    Controller,
    Get,
    Route,
} from "@tsoa/runtime";

import { ValidateErrorResponse, ValidateErrorStatus } from "../error_handler"
import { parseAuthToken } from "../util"

const HEADER_HTML = `
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
    <title>Forum</title>

    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css" integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2" crossorigin="anonymous">

    <style>
      body {
        padding-top: 5rem;
      }
      @media (min-width: 1200px) {
        .container {
            max-width: 1250px;
        }
      }
      .start-info {
        padding: 3rem 1.5rem;
        text-align: center;
      }
    </style>
  </head>
  <body>
  <script src="https://code.jquery.com/jquery-3.5.1.slim.min.js" integrity="sha384-DfXdz2htPH0lsSSs5nCTpuj/zy4C+OGpamoFVy38MVBnE+IbbVYUew+OrCXaRkfj" crossorigin="anonymous"></script>
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/js/bootstrap.bundle.min.js" integrity="sha384-ho+j7jyWK8fNQe+A12Hb8AhRq26LrZ/JpcUGGOn+Y7RsweNrtN/tE3MoK7ZeZDyx" crossorigin="anonymous"></script>
  <script src="//cdn.jsdelivr.net/npm/js-cookie@3.0.0-rc.1/dist/js.cookie.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/jstat@1.9.4/dist/jstat.min.js"></script>
  <script src="//cdn.plot.ly/plotly-1.57.0.min.js"></script>
  <script src="//unpkg.com/papaparse@5.3.0/papaparse.min.js"></script>
  <script>
  window.$ = document.querySelector.bind(document)
  
  const apiUrl = window.location.origin + '/app/polls'
  const userCookieName = 'user'
  
  function getRandomInt(min, max) {
      min = Math.ceil(min)
      max = Math.floor(max)
      return Math.floor(Math.random() * (max - min + 1)) + min
  }
  
  user = Cookies.get(userCookieName)
  if (!user) {
      user = 'joe' + getRandomInt(0, 1000).toString()
      Cookies.set(userCookieName, user)
  }

  document.addEventListener("DOMContentLoaded", () => {
    $('#user').innerHTML = user
  })
  
  function parseCSV(csv) {
      return Papa.parse(csv, {skipEmptyLines: true}).data
  }
  
  async function createPoll(topic, user, type) {
      const response = await fetch(apiUrl + '?topic=' + topic, {
          method: 'POST',
          headers: {
              'content-type': 'application/json',
              'authorization': 'Bearer user=' + user,
          },
          body: JSON.stringify({
              type: type
          })
      })
      if (!response.ok) {
          const error = await response.json()
          console.error(error)
          throw new Error('Could not create poll "' + topic + '": ' + error.message)
      }
  }
  
  async function submitOpinion(topic, user, opinion) {
      const response = await fetch(apiUrl + '?topic=' + topic, {
          method: 'PUT',
          headers: {
              'content-type': 'application/json',
              'authorization': 'Bearer user=' + user,
          },
          body: JSON.stringify({
              opinion: opinion
          })
      })
      if (!response.ok) {
          const error = await response.json()
          console.error(error)
          throw new Error('Could not submit opinion for poll "' + topic + '": ' + error.message)
      }
  }
  
  async function submitOpinions(user, opinions) {
      const response = await fetch(apiUrl + '/all', {
          method: 'PUT',
          headers: {
              'content-type': 'application/json',
              'authorization': 'Bearer user=' + user,
          },
          body: JSON.stringify(opinions)
      })
      if (!response.ok) {
          const error = await response.json()
          console.error(error)
          throw new Error('Could not submit opinions: ' + error.message)
      }
  }
  
  async function getPoll(topic, user) {
      const response = await fetch(apiUrl + '?topic=' + topic, {
          method: 'GET',
          headers: {
              'content-type': 'application/json',
              'authorization': 'Bearer user=' + user,
          }
      })
      if (!response.ok) {
          const error = await response.json()
          console.error(error)
          throw new Error('Could not retrieve poll "' + topic + '": ' + error.message)
      }
      const poll = await response.json()
      return poll
  }
  
  async function getPolls(user) {
      const response = await fetch(apiUrl + '/all', {
          method: 'GET',
          headers: {
              'content-type': 'application/json',
              'authorization': 'Bearer user=' + user,
          }
      })
      if (!response.ok) {
          const error = await response.json()
          console.error(error)
          throw new Error('Could not retrieve polls: ' + error.message)
      }
      const polls = await response.json()
      return polls
  }
  
  function plotPoll(element, topic, data) {
      if (!data.statistics) {
          plotEmptyPoll(element, topic)
      } else if (data.type == 'string') {
          plotStringPoll(element, topic, data)
      } else {
          plotNumberPoll(element, topic, data)
      }
  }
  
  const margin = {l: 30, r: 30, t: 30, b: 30}
  
  function plotNumberPoll(element, topic, data) {
      const mean = data.statistics.mean
      const std = data.statistics.std
      const normal = jStat.normal(mean, std)
      const xs = []
      const ys = []
      for (let i = mean - std*2; i < mean + std*2; i += 0.01) {
          xs.push(i)
          ys.push(normal.pdf(i))
      }
      
      const trace = {
          x: xs,
          y: ys,
          opacity: 0.5,
          line: {
              color: 'rgba(255, 0, 0)',
              width: 4
          },
          type: 'scatter'
      }
  
      const shapes = [{
          type: 'line',
          yref: 'paper',
          x0: mean,
          y0: 0,
          x1: mean,
          y1: 1,
          line:{
              color: 'black',
              width: 3,
          }
      }, {
          type: 'line',
          yref: 'paper',
          x0: mean - std,
          y0: 0,
          x1: mean - std,
          y1: 1,
          line:{
              color: 'black',
              width: 2,
          }
      }, {
          type: 'line',
          yref: 'paper',
          x0: mean + std,
          y0: 0,
          x1: mean + std,
          y1: 1,
          line:{
              color: 'black',
              width: 2,
          }
      }]
      if (data.opinion) {
          shapes.push({
              type: 'line',
              yref: 'paper',
              x0: data.opinion,
              y0: 0,
              x1: data.opinion,
              y1: 1,
              line:{
                  color: '#69c272',
                  width: 2,
              }
          })
      }
  
      Plotly.newPlot(element, [trace], {
          title: topic,
          shapes: shapes,
          xaxis: {
              zeroline: false
          },
          yaxis: {
              zeroline: false
          },
          margin: margin
        }, {displayModeBar: false})
  }
  
  function plotStringPoll(element, topic, data) {
      const strings = Object.keys(data.statistics.counts)
      const counts = Object.values(data.statistics.counts)
      const colors = strings.map(s => s == data.opinion ? '#69c272' : 'rgba(204,204,204,1)')
      const trace = {
          x: strings,
          y: counts,
          marker:{
              color: colors
          },
          type: 'bar'
      }
      Plotly.newPlot(element, [trace], {
          title: topic,
          margin: margin
      }, {displayModeBar: false})
  }
  
  function plotEmptyPoll(element, topic) {
      Plotly.newPlot(element, [], {
          title: topic,
          margin: margin,
          xaxis: {
            zeroline: false,
            showticklabels: false
          },
          yaxis: {
            zeroline: false,
            showticklabels: false
          },
          annotations: [{
            xref: 'paper',
            yref: 'paper',
            xanchor: 'center',
            yanchor: 'bottom',
            x: 0.5,
            y: 0.5,
            text: 'NOT ENOUGH DATA',
            showarrow: false
          }]
      }, {displayModeBar: false});
  }
  </script>

<nav class="navbar navbar-expand-md navbar-dark bg-dark fixed-top">
    <a class="navbar-brand" href="/app/site">Confidential Forum</a>
    <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarsExampleDefault" aria-controls="navbarsExampleDefault" aria-expanded="false" aria-label="Toggle navigation">
        <span class="navbar-toggler-icon"></span>
    </button>
    <div class="collapse navbar-collapse" id="navbarsExampleDefault">
        <ul class="navbar-nav mr-auto">
        <li class="nav-item">
            <a class="nav-link" href="/app/site/polls/create">Create Polls</a>
        </li>
        <li class="nav-item">
            <a class="nav-link" href="/app/site/opinions/submit">Submit Opinions</a>
        </li>
        <li class="nav-item">
            <a class="nav-link" href="/app/site/view">View Statistics</a>
        </li>
        </ul>
        <span class="navbar-text">
           User: <span id="user"></span>
        </span>
    </div>
</nav>
`

const FOOTER_HTML = `
</body>
</html>
`

const START_HTML = `
${HEADER_HTML}

<main role="main" class="container">

  <div class="start-info">
    <h1>Confidential Forum</h1>
    <p class="lead">Blabla<br>Blablabla.</p>
  </div>

</main>

${FOOTER_HTML}
`

const CREATE_POLLS_HTML = `
${HEADER_HTML}

<main role="main" class="container">

    <textarea id="input-polls" rows="10" cols="70" placeholder="My Topic,string&#10;My other topic,number"></textarea>
    <br />
    <button id="create-polls-btn" class="btn btn-primary">Create Polls</button>

</main>

<script>
$('#create-polls-btn').addEventListener('click', async () => {
    const lines = parseCSV($('#input-polls').value)
    for (const [topic, type] of lines) {
        try {
            await createPoll(topic, user, type)
        } catch (e) {
            window.alert(e)
            return
        }
    }
    window.alert('Successfully created polls.')
    $('#input-polls').value = ''
})

</script>

${FOOTER_HTML}
`

const SUBMIT_OPINIONS_HTML = `
${HEADER_HTML}

<main role="main" class="container">

    <textarea id="input-opinions" rows="10" cols="70" placeholder="My Topic,abc&#10;My other topic,3.5"></textarea>
    <br />
    <button id="submit-opinions-btn" class="btn btn-primary">Submit Opinions</button>

</main>

<script>
$('#submit-opinions-btn').addEventListener('click', async () => {
    const lines = parseCSV($('#input-opinions').value)
    const opinions = {}
    for (let [topic, opinion] of lines) {
        if (!Number.isNaN(Number(opinion))) {
            opinion = parseFloat(opinion)
        }
        opinions[topic] = {opinion: opinion}
    }
    try {
        await submitOpinions(user, opinions)
    } catch (e) {
        window.alert(e)
        return
    }
    window.alert('Successfully submitted opinions.')
    $('#input-opinions').value = ''
})

</script>
${FOOTER_HTML}
`

const VIEW_HTML = `
${HEADER_HTML}
<style>
.plot {
    width: 300px;
    height: 150px;
    float: left;
}
</style>

<main role="main" class="container">
    <div id="plots"></div>
</main>

<script>
async function main() {
    const polls = await getPolls(user)
    const topics = Object.keys(polls)

    const plotsEl = $('#plots')
    plotsEl.innerHTML = topics.map((topic,i) => '<div class="plot" id="plot_' + i + '"></div>').join('')
    for (let [i, topic] of topics.entries()) {
        plotPoll('plot_' + i, topic, polls[topic])
    }
}
main()

</script>
${FOOTER_HTML}
`

const HTML_CONTENT_TYPE = 'text/html'

@Hidden()
@Route("site")
export class SiteController extends Controller {

    @Get()
    public getStartPage(): any {
        this.setHeader('content-type', HTML_CONTENT_TYPE)
        return START_HTML
    }

    @Get('polls/create')
    public getPollsCreatePage(): any {
        this.setHeader('content-type', HTML_CONTENT_TYPE)
        return CREATE_POLLS_HTML
    }

    @Get('opinions/submit')
    public getOpinionsSubmitPage(): any {
        this.setHeader('content-type', HTML_CONTENT_TYPE)
        return SUBMIT_OPINIONS_HTML
    }

    @Get('view')
    public getViewPage(): any {
        this.setHeader('content-type', HTML_CONTENT_TYPE)
        return VIEW_HTML
    }
}