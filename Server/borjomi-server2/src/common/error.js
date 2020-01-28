
const ErrorCodes = {
    ConnectionClosed: -1,
    Ok: 0,
    HandshakeRequired: 1,
    InvalidFormat: 2,
    FunctionNotFound: 3,
    InvalidParams: 4,
    GeneralError: 5
}

class APIError extends Error {
    constructor(message, code = ErrorCodes.Ok) {
        super(message)
        this.name = "APIError"
        this.code = code
    }
}

export { ErrorCodes, APIError }