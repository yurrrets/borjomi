const ErrorCodes = { 
    Ok : 0,
    GeneralError : 1,
    HandshakeRequired : 2,
    FunctionNotFound : 3
}

class APIError extends Error {
    constructor(message, code = ErrorCodes.OK) {
        super(message)
        this.name = "APIError"
        this.code = code
    }
}

export { ErrorCodes, APIError }