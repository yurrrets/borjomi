
const MessageStatus = {
    New: 1,            // New message
    Cancelled: 2,      // Cancelled by requestor
    Accepted: 3,       // Accepted by executor
    Processing: 4,     // Executing in process
    DoneOk: 5,         // Completed successfully
    DoneError: 6,      // Message processed, but error happened (most probably not completed)
    TimedOut: 7,       // Message hasn't been accepted or processed in time
}

const MessageType = {
    /**
     * Ping request
     */
    Ping: "ping",

}

class Message {
    constructor() {
        this.id = 0               // unique id
        this.status = 0           // current message status; see enum above
        this.type = null          // message type; see enum above
        this.creationDate = null  // when message was created
        this.lastModified = null  // datetime of last modification
        this.requestor = 0        // requestor (account id)
        this.target = 0           // executor (account id); 0 means that executor is server itself, and it should process the message
        this.validUntil = null    // datetime; null means always valid no-timeout message
        this.params = null        // object; message parameters
    }
}

class MessageAnswer {
    constructor() {
        this.messageId = 0        // id of message
        this.errorCode = 0        // {error.ErrorCodes} error code
        this.errorText = null     // text description of error
        this.notes = null         // additional info
    }
}

export { MessageStatus, MessageType, Message, MessageAnswer }