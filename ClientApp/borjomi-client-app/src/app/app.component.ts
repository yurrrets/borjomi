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
import { AuthService } from './core/services/auth.service';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})

export class AppComponent {   
  
  constructor(private auth: AuthService) {
    this.auth.logout(); 
  }
  

  

  
   

  
}
