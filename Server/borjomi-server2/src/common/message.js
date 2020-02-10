
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
     * Ping request. Should be answered with "pong".
     * @param requestor should be valid
     * @param target could be child account id, or -1 for multicasting
     */
    Ping: "ping",

    /**
     * Pong answer to "ping" request.
     * @param parentMessageId id of "ping" message
     * @param target id of requestor from "ping" message
     */
    Pong: "pong"
}

class Message {
    constructor() {
        this.id = 0               // unique id
        this.parentMessageId = 0  // used for indicating that this message is an anwer to some other message
        this.status = 0           // current message status; see enum above
        this.type = null          // message type; see enum above
        this.creationDate = null  // when message was created
        this.lastModified = null  // datetime of last modification
        this.requestor = 0        // requestor (account id)
        this.target = 0           // executor (account id);
                                  // 0 means that target is server, it shouldn't be sent to anyone;
                                  // -1 means multicast message, sent to all child accounts of requestor
        this.validUntil = null    // datetime; null means always valid no-timeout message
        this.params = null        // object; message parameters
    }
}

export { MessageStatus, MessageType, Message }