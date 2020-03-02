
const MessageStatus = {
    New: 1,            // New message
    Sent: 2,           // Message was sent to executor
    Accepted: 3,       // Accepted by executor
    Processing: 4,     // Executing in process
    DoneOk: 5,         // Completed successfully
    DoneError: 6,      // Message processed, but error happened (most probably not completed)
    TimedOut: 7,       // Message hasn't been accepted or processed in time
}

// Message workflow states.
// Described as state machine. In brakets is a role who can change to given state
// New -> Sent, TimedOut (server)
// Sent -> New (server) | Accepted, Processing, DoneOk, DoneError (executor)
// Accepted -> Processing, DoneOk, DoneError (executor)
// Processing -> DoneOk, DoneError (executor)

class Message {
    constructor() {
        this.id = 0               // unique id
        this.status = 0           // current message status; see enum above
        this.type = null          // message type; see enum above
        this.creationDate = null  // when message was created
        /** @type {Date} */
        this.lastModified = null  // datetime of last modification
        this.requestor = 0        // requestor (account id)
        this.executor = 0         // executor (account id); 0 means that executor is server itself, and it should process the message
        this.validUntil = null    // datetime; null means always valid no-timeout message
        this.params = null        // object; message parameters
    }
}

class MessageAnswer {
    constructor() {
        this.errorCode = 0        // {error.ErrorCodes} error code
        this.errorText = null     // text description of error
        this.result = null        // additional info
    }
}

export { MessageStatus, Message, MessageAnswer }