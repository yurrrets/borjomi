import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { environment } from 'src/environments/environment';
import { WebSocketSubject, webSocket } from 'rxjs/webSocket';
import { map, switchMap, delay, retryWhen, filter } from 'rxjs/operators';


//const SERVER_URL = "ws://localhost:3001";
@Injectable()
export class WebSocketService{
    ws: WebSocket;
    connection$: WebSocketSubject<any>;
    RETRY_SECONDS = 10; 
    serverUrl: string = environment.wsUrl;
    socketIsOpen = 1;     
    store: Array<string>;
   
    connect(): Observable<any> {
        this.ws = new WebSocket(this.serverUrl);
        console.log("ws connect");

        return new Observable(
           observer => {
    
            this.ws.onmessage = (event) =>
            {
                observer.next(event.data);
                console.log(event.data);
            }
    
            this.ws.onerror = (event) => observer.error(event);
    
            this.ws.onclose = (event) => observer.complete();
    
            return () =>
                this.ws.close(1000, "The user disconnected");
           }
        );
    }   

   /* connect(): Observable<any> {
        return new Observable(      observer => {    
          switchMap(wsUrl => {
            if (this.connection$) {
              return this.connection$;
            } else {
              this.connection$ = webSocket(this.serverUrl);
              return this.connection$;
            }
          })
          //retryWhen((errors) => errors.pipe(delay(this.RETRY_SECONDS)))
        });
      }*/
    
    /*sendMessage(message: any) {
        if(this.connection$){
            this.connection$.next(message);
        }
        else{
            console.error('Did not send data, open a connection first');
        }
    }  */
    sendMessage(message: any): string {
        if(this.ws.readyState === this.socketIsOpen){
            this.ws.send(message);
            return `Sent to server ${message}`;
        }
        else{
            return "Message was not sent";
        }
    }  
}