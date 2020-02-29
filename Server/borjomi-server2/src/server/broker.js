import { mergeDeep } from '../common/utils'


const DefaultBrokerParams = {
    callTimeout: 0, // ms
}

class Broker {
    constructor(params) {
        this.params = mergeDeep({}, DefaultBrokerParams, params)
        this._newMessageRequestFlag = false
        this._newMessageProcessingFlag = false
        this.newMessageHandler = null
        this.messageStatusChanged = null
        this.messageAnswerHandler = null
    }

    notifyNewMessage() {
        this._newMessageRequestFlag = true
        const self = this
        setTimeout(async function() {
            try { await self._handleNewMessage() }
            catch(e) { throw e } // add handling
        }, this.params.callTimeout)
    }

    async _handleNewMessage() {
        if (this._newMessageProcessingFlag) {
            return
        }
        this._newMessageProcessingFlag = true
        while (this._newMessageRequestFlag) { // new requests could appear while processing handler, that is why cycle here
            this._newMessageRequestFlag = false
            if (this.newMessageHandler) {
                await this.newMessageHandler()
            }
        }
        this._newMessageProcessingFlag = false
    }

    notifyMessageStatusChanged(msgId) {
        const self = this
        setTimeout(async function() {
            if (!self.messageStatusChanged) {
                return
            }
            await self.messageStatusChanged(msgId)
        }, this.params.callTimeout)
    }

    notifyMessageAnswer(msgId) {
        const self = this
        setTimeout(async function() {
            if (!self.messageAnswerHandler) {
                return
            }
            await self.messageAnswerHandler(msgId)
        }, this.params.callTimeout)
    }
}

export { Broker }