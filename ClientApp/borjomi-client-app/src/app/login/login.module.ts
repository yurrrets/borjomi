import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LoginComponent } from './login.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MaterialModule } from '../core/material/material.module';
import { MatDialogRef, MatDialogModule } from '@angular/material/dialog';
import { WebSocketService } from '../core/services/websocket.service';



@NgModule({
  declarations: [
    LoginComponent
  ],
  imports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,
    MaterialModule
  ],
  providers: [
    WebSocketService, 
    MatDialogModule,
    {
    provide: MatDialogRef,
    useValue: {}
  }],
  exports: [    
    LoginComponent  
  ]
})
export class LoginModule { }
