import { mergeDeep } from '../common/utils'


const DefaultBrokerParams = {
    callTimeout: 10, // ms
}

class Broker {
    constructor(wsServer, params) {
        this.wsServer = wsServer
        this.params = mergeDeep({}, DefaultBrokerParams, params)
        this.newMessageHandler = null
        this.newAnswerHandler = null
    }

    notifyNewMessage(messageId, executorId) {
        setTimeout(function () {
            if (this.newMessageHandler) {
                this.newMessageHandler(this.wsServer, messageId, executorId)
            }
        }, this.params.callTimeout)
    }
}