import { KJUR, KEYUTIL, ArrayBuffertohex } from 'jsrsasign'
import jwt_decode from 'jwt-decode'
import * as msgpack from "@msgpack/msgpack"
import { Base64 } from 'js-base64'

import * as ccf from '../types/ccf'

interface JwtResponse {
    userId: string
}

interface ErrorResponse {
    msg: string
}

interface HeaderClaims {
    kid: string
}

interface BodyClaims {
    sub: string
}

class GovConverter<T> implements ccf.DataConverter<T> {
    encode(val: T): ArrayBuffer {
        const ta = msgpack.encode(val)
        return ta.buffer.slice(ta.byteOffset, ta.byteOffset + ta.byteLength);;
    }
    decode(buf: ArrayBuffer): T {
        return msgpack.decode(buf) as T;
    }
}
const govString = new GovConverter<string>()
const govUint8Array = new GovConverter<number[]>()

export function jwt(request: ccf.Request): ccf.Response<JwtResponse | ErrorResponse> {
    const authHeader = request.headers['authorization']
    if (!authHeader) {
        return unauthorized('authorization header missing')
    }
    const parts = authHeader.split(' ', 2)
        if (parts.length !== 2 || parts[0] !== 'Bearer') {
            return unauthorized('unexpected authentication type')
        }
    const token = parts[1]

    // Extract header claims to select the correct signing key.
    // We use jwt_decode() instead of jsrsasign's parse() as the latter does unnecessary work.
    let headerClaims: HeaderClaims
    try {
        headerClaims = jwt_decode<HeaderClaims>(token, { header: true })
    } catch (e) {
        return unauthorized(`malformed jwt: ${e.message}`)
    }
    const signingKeyId = headerClaims.kid
    if (!signingKeyId) {
        return unauthorized('kid missing in header claims')
    }

    // Get the stored signing key to validate the token.
    const keysMap = new ccf.TypedKVMap(ccf.kv['public:ccf.gov.jwt_public_signing_keys'], govString, govUint8Array)
    const publicKeyDer = keysMap.get(signingKeyId)
    if (publicKeyDer === undefined) {
        return unauthorized(`token signing key not found: ${signingKeyId}`)
    }
    // jsrsasign can only load X.509 certs from PEM strings
    const publicKeyB64 = Base64.fromUint8Array(new Uint8Array(publicKeyDer))
    const publicKeyPem = "-----BEGIN CERTIFICATE-----\n" + publicKeyB64 + "\n-----END CERTIFICATE-----";
    const publicKey = KEYUTIL.getKey(publicKeyPem)

    // Check whether the issuer needs to be validated.
    const validateIssuerMap = new ccf.TypedKVMap(ccf.kv['public:ccf.gov.jwt_public_signing_keys_validate_issuer'],
        govString, govString)
    const validateIssuer = validateIssuerMap.get(signingKeyId)
    const expectedIssuer = validateIssuer ? [validateIssuer] : undefined

    // Validate the token signature and issuer.
    const valid = KJUR.jws.JWS.verifyJWT(token, <any>publicKey, <any>{
        alg: ['RS256'],
        iss: expectedIssuer,
        // No trusted time, disable time validation.
        verifyAt: Date.parse('2020-01-01T00:00:00') / 1000,
        gracePeriod: 10 * 365 * 24 * 60 * 60
    })
    if (!valid) {
        return unauthorized('jwt validation failed')
    }

    // Custom body claims validation, app-specific.
    const claims = jwt_decode<BodyClaims>(token)
    if (!claims.sub) {
        return unauthorized('jwt invalid, sub claim missing')
    }
    return {
        body: {
            userId: claims.sub
        }
    }
}

function unauthorized(msg: string): ccf.Response<ErrorResponse> {
    return {
        statusCode: 401,
        body: {
            msg: msg
        }
    }
}