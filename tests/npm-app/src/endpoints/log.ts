import { ccf, Request, Response } from "../ccf/builtin";
import * as ccfUtil from "../ccf/util";

interface LogItem {
  msg: string;
}

interface LogEntry extends LogItem {
  id: number;
}

const logMap = new ccfUtil.TypedKVMap(
  ccf.kv.log,
  ccfUtil.uint32,
  ccfUtil.json<LogItem>()
);

export function getLogItem(request: Request): Response<LogItem> {
  const id = parseInt(request.query.split("=")[1]);
  if (!logMap.has(id)) {
    return {
      statusCode: 404,
    };
  }
  return {
    body: logMap.get(id),
  };
}

export function setLogItem(request: Request<LogItem>): Response {
  const id = parseInt(request.query.split("=")[1]);
  logMap.set(id, request.body.json());
  return {};
}

export function getAllLogItems(
  request: Request
): Response<Array<LogEntry>> {
  let items: Array<LogEntry> = [];
  logMap.forEach(function (item, id, table) {
    items.push({ id: id, msg: item.msg });
  });
  return {
    body: items,
  };
}
