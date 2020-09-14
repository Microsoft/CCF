import * as _ from 'lodash-es'
import * as rs  from 'jsrsasign';
// Importing the browser bundle works around https://github.com/protobufjs/protobuf.js/issues/1402.
import protobuf from 'protobufjs/dist/protobuf.js'

declare var body: {
    text: () => string;
    json: () => any;
    arrayBuffer: () => ArrayBuffer;
};

type PartitionRequest = [any]
type PartitionResponse = [any[], any[]]

export function partition(): PartitionResponse {
    // Example from https://lodash.com.
    let arr: PartitionRequest = body.json();
    return _.partition(arr, n => n % 2);
}

type ProtoResponse = Uint8Array

export function proto(): ProtoResponse {
    // Example from https://github.com/protobufjs/protobuf.js.
    let Type  = protobuf.Type;
    let Field = protobuf.Field;
 
    let AwesomeMessage = new Type("AwesomeMessage").add(new Field("awesomeField", 1, "string"));
    
    let message = AwesomeMessage.create({ awesomeField: body.text() });
    let arr = AwesomeMessage.encode(message).finish();
    return arr;
}

interface CryptoResponse {
    available: boolean
}

export function crypto(): CryptoResponse {
    // Most functionality of jsrsasign requires keys.
    // Generating a key here is too slow, so we'll just check if the
    // JS API got exported correctly.
    if (rs.KEYUTIL.generateKeypair) {
        return { available: true };
    } else {
        return { available: false };
    }
}
