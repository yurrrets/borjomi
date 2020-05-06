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
  dataSubscription: Subscription;  
    
  constructor(private wsService: WebSocketService,
      private router: Router ) {}  
    
  ngOnInit(): void {
    if (this.dataSubscription) {
      this.dataSubscription.unsubscribe();
    }

    this.dataSubscription = this.wsService.getChildAccounts()
      .subscribe(data => {
        console.log(data);
      
      
      },
      err => console.log(err),
      () =>  console.log( 'The observable stream is complete'))
    ;
  } 
    
    
  
   
}