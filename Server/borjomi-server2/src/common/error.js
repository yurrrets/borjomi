
const ErrorCodes = {
    ConnectionClosed: -1,
    Ok: 0,
    GeneralError: 1,
    HandshakeRequired: 2,
    NotLoggedIn: 3,
    InvalidFormat: 4,
    FunctionNotFound: 5,
    InvalidParams: 6
}

class APIError extends Error {
    constructor(message, code = ErrorCodes.Ok) {
        super(message)
        this.name = "APIError"
        this.code = code
    }
}

export { ErrorCodes, APIError }