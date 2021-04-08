import { assert } from "chai";
import * as crypto from "crypto";
import "../src/polyfill";
import {
  AesKwpParams,
  ccf,
  RsaOaepAesKwpParams,
  RsaOaepParams,
} from "../src/global";
import { unwrapKey, generateSelfSignedCert } from "./crypto";

beforeEach(function () {
  // clear KV before each test
  for (const prop of Object.getOwnPropertyNames(ccf.kv)) {
    delete ccf.kv[prop];
  }
});

describe("polyfill", function () {
  describe("strToBuf/bufToStr", function () {
    it("converts string <--> ArrayBuffer", function () {
      const s = "foo";
      assert.equal(ccf.bufToStr(ccf.strToBuf(s)), s);
    });
  });
  describe("jsonCompatibleToBuf/bufToJsonCompatible", function () {
    it("converts JSON-compatible <--> ArrayBuffer", function () {
      const s = { foo: "bar" };
      assert.deepEqual(ccf.bufToJsonCompatible(ccf.jsonCompatibleToBuf(s)), s);
    });
  });
  describe("generateAesKey", function () {
    it("generates a random AES key", function () {
      assert.equal(ccf.generateAesKey(128).byteLength, 16);
      assert.equal(ccf.generateAesKey(192).byteLength, 24);
      assert.equal(ccf.generateAesKey(256).byteLength, 32);
      assert.notDeepEqual(ccf.generateAesKey(256), ccf.generateAesKey(256));
    });
  });
  describe("generateRsaKeyPair", function () {
    it("generates a random RSA key pair", function () {
      const pair = ccf.generateRsaKeyPair(2048);
      assert.isTrue(pair.publicKey.startsWith("-----BEGIN PUBLIC KEY-----"));
      assert.isTrue(pair.privateKey.startsWith("-----BEGIN PRIVATE KEY-----"));
    });
  });
  describe("wrapKey", function () {
    it("performs RSA-OAEP wrapping correctly", function () {
      const key = ccf.generateAesKey(128);
      const wrappingKey = ccf.generateRsaKeyPair(2048);
      const wrapAlgo: RsaOaepParams = {
        name: "RSA-OAEP",
      };
      const wrapped = ccf.wrapKey(
        key,
        ccf.strToBuf(wrappingKey.publicKey),
        wrapAlgo
      );
      const unwrapped = unwrapKey(
        wrapped,
        ccf.strToBuf(wrappingKey.privateKey),
        wrapAlgo
      );
      assert.deepEqual(unwrapped, key);
    });
    it("performs AES-KWP wrapping correctly", function () {
      const key = ccf.generateAesKey(128);
      const wrappingKey = ccf.generateAesKey(256);
      const wrapAlgo: AesKwpParams = {
        name: "AES-KWP",
      };
      const wrapped = ccf.wrapKey(key, wrappingKey, wrapAlgo);
      const unwrapped = unwrapKey(wrapped, wrappingKey, wrapAlgo);
      assert.deepEqual(unwrapped, key);
    });
    it("performs RSA-OAEP-AES-KWP wrapping correctly", function () {
      const key = ccf.generateAesKey(128);
      const wrappingKey = ccf.generateRsaKeyPair(2048);
      const wrapAlgo: RsaOaepAesKwpParams = {
        name: "RSA-OAEP-AES-KWP",
        aesKeySize: 256,
      };
      const wrapped = ccf.wrapKey(
        key,
        ccf.strToBuf(wrappingKey.publicKey),
        wrapAlgo
      );
      const unwrapped = unwrapKey(
        wrapped,
        ccf.strToBuf(wrappingKey.privateKey),
        wrapAlgo
      );
      assert.deepEqual(unwrapped, key);
    });
  });

  describe("isValidX509CertBundle", function (this) {
    const supported = "X509Certificate" in crypto;
    it("returns true for valid certs", function () {
      if (!supported) {
        this.skip();
      }
      const pem1 = generateSelfSignedCert();
      const pem2 = generateSelfSignedCert();
      assert.isTrue(ccf.isValidX509CertBundle(pem1));
      assert.isTrue(ccf.isValidX509CertBundle(pem1 + "\n" + pem2));
    });
    it("returns false for invalid certs", function () {
      if (!supported) {
        this.skip();
      }
      assert.isFalse(ccf.isValidX509CertBundle("garbage"));
    });
  });
  describe("kv", function () {
    it("basic", function () {
      const foo = ccf.kv["foo"];

      const key = "bar";
      const val = 65535;
      const key_buf = ccf.strToBuf(key);
      const val_buf = ccf.jsonCompatibleToBuf(val);

      assert.equal(foo.get(key_buf), undefined);

      foo.set(key_buf, val_buf);
      assert.deepEqual(foo.get(key_buf), val_buf);
      assert.isTrue(foo.has(key_buf));

      let found = false;
      foo.forEach((v, k) => {
        if (ccf.bufToStr(k) == key && ccf.bufToJsonCompatible(v) == val) {
          found = true;
        }
      });
      assert.isTrue(found);

      foo.delete(key_buf);
      assert.isNotTrue(foo.has(key_buf));
      assert.equal(foo.get(key_buf), undefined);
    });
  });
});
