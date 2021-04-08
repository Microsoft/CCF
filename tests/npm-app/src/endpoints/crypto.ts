import * as rs from "jsrsasign";
import { Base64 } from "js-base64";

import * as ccfapp from "@microsoft/ccf-app";
import * as ccfcrypto from "@microsoft/ccf-app/crypto";

interface CryptoResponse {
  available: boolean;
}

export function crypto(
  request: ccfapp.Request
): ccfapp.Response<CryptoResponse> {
  // Most functionality of jsrsasign requires keys.
  // Generating a key here is too slow, so we'll just check if the
  // JS API got exported correctly.
  let available = rs.KEYUTIL.generateKeypair ? true : false;
  return { body: { available: available } };
}

interface GenerateAesKeyRequest {
  size: number;
}

export function generateAesKey(
  request: ccfapp.Request<GenerateAesKeyRequest>
): ccfapp.Response<ArrayBuffer> {
  return { body: ccfcrypto.generateAesKey(request.body.json().size) };
}

interface GenerateRsaKeyPairRequest {
  size: number;
  exponent?: number;
}

export interface GenerateRsaKeyPairResponse {
  privateKey: string;
  publicKey: string;
}

export function generateRsaKeyPair(
  request: ccfapp.Request<GenerateRsaKeyPairRequest>
): ccfapp.Response<GenerateRsaKeyPairResponse> {
  const req = request.body.json();
  const res = req.exponent
    ? ccfcrypto.generateRsaKeyPair(req.size, req.exponent)
    : ccfcrypto.generateRsaKeyPair(req.size);
  return { body: res };
}

type Base64 = string;

interface RsaOaepParams {
  name: "RSA-OAEP";
  label?: Base64;
}

interface RsaOaepAesKwpParams {
  name: "RSA-OAEP-AES-KWP";
  aesKeySize: number; // in bits
  label?: Base64;
}

type WrapAlgoParams =
  | RsaOaepParams
  | RsaOaepAesKwpParams
  | ccfcrypto.AesKwpParams;

interface WrapKeyRequest {
  key: Base64; // typically an AES key
  wrappingKey: Base64; // base64 encoding of PEM-encoded RSA public key or AES key bytes
  wrapAlgo: WrapAlgoParams; // Wrapping algorithm parameters
}

export function wrapKey(
  request: ccfapp.Request<WrapKeyRequest>
): ccfapp.Response<ArrayBuffer> {
  const r = request.body.json();
  const key = b64ToBuf(r.key);
  const wrappingKey = b64ToBuf(r.wrappingKey);
  let wrappedKey: ArrayBuffer;
  if (r.wrapAlgo.name == "RSA-OAEP") {
    const label = r.wrapAlgo.label ? b64ToBuf(r.wrapAlgo.label) : undefined;
    wrappedKey = ccfcrypto.wrapKey(key, wrappingKey, {
      name: r.wrapAlgo.name,
      label: label,
    });
  } else if (r.wrapAlgo.name == "RSA-OAEP-AES-KWP") {
    const label = r.wrapAlgo.label ? b64ToBuf(r.wrapAlgo.label) : undefined;
    wrappedKey = ccfcrypto.wrapKey(key, wrappingKey, {
      name: r.wrapAlgo.name,
      aesKeySize: r.wrapAlgo.aesKeySize,
      label: label,
    });
  } else {
    wrappedKey = ccfcrypto.wrapKey(key, wrappingKey, r.wrapAlgo);
  }
  return { body: wrappedKey };
}

export function isValidX509CertBundle(
  request: ccfapp.Request
): ccfapp.Response<boolean> {
  const pem = request.body.text();
  return { body: ccfcrypto.isValidX509CertBundle(pem) };
}

function b64ToBuf(b64: string): ArrayBuffer {
  return Base64.toUint8Array(b64).buffer;
}
