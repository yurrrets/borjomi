import { Component, OnInit } from '@angular/core';
import { WebSocketService } from '../core/services/websocket.service';
import { Router } from '@angular/router';
import { Subscription } from 'rxjs';

@Component({
    selector: 'borjomi-home',
    templateUrl: 'home.component.html',
    styleUrls: []
  })
  
export class HomeComponent implements OnInit {
  wsSubscription: Subscription;  
    
  constructor(private wsService: WebSocketService,
      private router: Router ) {}  
    
  ngOnInit(): void {
      this.initIoConnection(); 
  } 
    
  initIoConnection() { 
    this.wsSubscription =
      this.wsService.connect()
      .subscribe(
          message => {          
          console.log(message); 
              this.onSocketMessage(message);  
          },
          err => console.log( 'err'),
          () => console.log( 'The observable stream is complete')
      ); 
         
  }
  onSocketMessage(message: any) {
    throw new Error("Method not implemented.");
  }
    
  closeSocket(){
    this.wsSubscription.unsubscribe();
  }

  ngOnDestroy() {
    this.closeSocket();  
  }
   
}