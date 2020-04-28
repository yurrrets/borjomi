import { Component, ElementRef, ViewChild, AfterViewInit, OnInit, OnDestroy } from '@angular/core';
import { MatDialogRef, MatDialog } from '@angular/material/dialog';
import { LoginComponent } from './login/login.component';
import {webSocket} from 'rxjs/webSocket';
import { WebSocketService } from './core/services/websocket.service';
import { IMessage } from './core/model/message';
import { ServerEvent } from './core/model/event';
import { Func } from './core/model/function';
import { Subject, Subscription } from 'rxjs';
import { takeUntil } from 'rxjs/operators';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})

export class AppComponent implements OnInit, OnDestroy {  
  title = 'borjomi-client-app';
  //subject = webSocket('ws://localhost:3001/');
  messages: IMessage[] = [];
  message = 'Hello';

  messageFromServer: string;
  wsSubscription: Subscription;
  status;

  


  messageContent: string;
  ioConnection: any;
  dialogRef: MatDialogRef<LoginComponent> | null;
  defaultDialogUserParams: any = {
    disableClose: true,
    data: {
      title: 'Welcome',
      title_pt: 'Bem-vindo'
    }
  };
  
  constructor(private wsService: WebSocketService, public dialog: MatDialog) {
    
  }
 
  sendMessageToServer(){
    this.status = this.wsService.sendMessage("Hello from client");
  }

  closeSocket(){
    this.wsSubscription.unsubscribe();
    this.status = 'The socket is closed';
  }

  ngOnDestroy() {
    this.closeSocket();
  }

  ngOnInit(): void {
    this.wsSubscription =
      this.wsService.connect()
       .subscribe(
        data => {
          this.messageFromServer = data;
          console.log(data);
        },
        err => console.log( 'err'),
        () =>  console.log( 'The observable stream is complete')
      );

    setTimeout(() => {
      this.openUserPopup(this.defaultDialogUserParams);
    }, 0);
    
  }
  
  private openUserPopup(params): void {
    this.dialogRef = this.dialog.open(LoginComponent, params);
    this.dialogRef.afterClosed().subscribe(paramsDialog => {
      if (!paramsDialog) {
        return;
      }   
      
      this.sendNotification(1, Func.HANDSHAKE);      
      this.sendNotification(paramsDialog, Func.LOGIN);
    });
    
  }

  /*private initIoConnection(): void {
    this.subject.subscribe(msg=> {
      console.log(msg);
    });
    
  }*/

  
  public sendNotification(params: any, func: Func): void {
    let message: IMessage;

    switch(func){
      case Func.HANDSHAKE:
        message = {
          function: Func.HANDSHAKE,          
          version: 1,
          reqID: 1
        };
        break;
      case Func.LOGIN:
        message = {
          function: Func.LOGIN,          
          username: params.username,
          password: params.password
        };
        break;
      default:
        break;
    }    
   
    let lmsg = this.wsService.sendMessage(JSON.stringify(message));
    console.log(lmsg);
  }
}
