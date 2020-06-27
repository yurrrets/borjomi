import { Injectable, EventEmitter } from '@angular/core';
import { Observable, Subject} from 'rxjs';
import { environment } from 'src/environments/environment';
import { webSocket, WebSocketSubject } from 'rxjs/webSocket';
 
const wsSubject = webSocket({
    url: environment.wsUrl,
    protocol: 'ws',
    serializer: msg => JSON.stringify({
        //channel: "webDevelopment", 
        msg: msg}),
    deserializer: ({data}) => {
        let obj = JSON.parse(data);
        if (!obj) return;
        return obj;
    },
    openObserver: {
        next: () => console.log('Underlying WebSocket connection open')
    } 
});

//const SERVER_URL = "ws://localhost:3001";

@Injectable()
export class WebSocketService{      
    serverUrl: string = environment.wsUrl;
    webSocketSubject: any;
    connected: boolean = false;
    networkError: boolean = false;
    msgId: number;

    private connect(): WebSocketSubject<any> {      
        if (!this.webSocketSubject)  
        {          
            const closeSubject = new Subject<CloseEvent>();
            closeSubject.subscribe(_ => {
              this.webSocketSubject = null;
              if (this.connected) {
                this.networkError = true;
              }
            });
      
            this.webSocketSubject = webSocket({
              url: this.serverUrl,            
              protocol: 'ws',
              serializer: msg => {
                  return JSON.stringify(msg, (k,val)=>{
                      if (typeof val === 'number'){
                      if (isFinite(val)) return val;
                      return {_dtype:'number', value: val.toString() }
                      }                      
                      return val;
                  });
              },
              deserializer: ({data}) => {
                  let obj = JSON.parse(data);
                  if (!obj) return;
                  return obj;
              },
              openObserver: {
                  next: () => console.log('Underlying WebSocket connection open')
              },   
              closeObserver: closeSubject       
            });
        }
        return this.webSocketSubject;
    }

    async handshake() {
        let zzz = this.connect().multiplex(() => 'subscribe-handshake', () => 'unsubscribe-handshake', message => message.version)
        zzz.subscribe(msg=> console.log(msg));
        this.connect().next({
            'function': 'Handshake',
            'version': 1
        });      
        zzz.subscribe();        
    }

    login(obj: any){
        let zzz = this.connect().multiplex(() => 'subscribe-login', () => 'unsubscribe-login', message => message.token);
        this.connect().next(obj);       
        console.log(zzz);
        return zzz;
    }

    getChildAccounts() {
        let zzz = this.connect().multiplex(() => 'subscribe-getChildAccounts', () => 'unsubscribe-getChildAccounts', message => message.childAccounts);
        this.connect().next({'function': "getChildAccounts"});       
        console.log(zzz);
        return zzz;
        
    }
        
  }