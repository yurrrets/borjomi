import { Injectable } from '@angular/core';
import { Observable} from 'rxjs';
import { environment } from 'src/environments/environment';
import { Func } from '../model/function';

//const SERVER_URL = "ws://localhost:3001";

@Injectable()
export class WebSocketService{
    ws: WebSocket;
    serverUrl: string = environment.wsUrl;
   
    connect(): Observable<any> {  
        if ( this.ws && this.ws.readyState === this.ws.OPEN )
                this.ws.close();         
        this.ws = new WebSocket(this.serverUrl);
        console.log("connecting to...");       
       
        return new Observable(
           observer => { 
            this.ws.onopen = (event) =>{
                console.log("CONNECTED");
                let message = {function: Func.HANDSHAKE,version: 1,reqID: 1};
                this.sendMessage(this.customJSONStringify(message));
            };  
            this.ws.onmessage = (event) =>observer.next(event.data);  
              
            this.ws.onerror = (event) => observer.error(event);

            this.ws.onclose = (event) => observer.complete();
    
            //return () => this.ws.close(1000, "The user disconnected");
        });      
        
    }       
  
    sendMessage(message: any) {
        if(this.ws.readyState !== this.ws.OPEN)
            return "Message was not sent";

        this.ws.send(message);
        return `Sent to server ${message}`;
    }  

    customJSONStringify(obj: any) { 
        return JSON.stringify(obj, (k,val)=>{
            if (typeof val === 'number'){
              if (isFinite(val)) return val;
              return {_dtype:'number', value: val.toString() }
            }
            return val;
        })
    }
        
  }