import { Component, OnInit, OnDestroy } from '@angular/core';
import { FormControl, Validators, FormGroup } from '@angular/forms';
import { WebSocketService } from '../core/services/websocket.service';
import { AuthService } from '../core/services/auth.service';
import { Router } from '@angular/router';
import { Func } from '../core/model/function';
import { Subscription } from 'rxjs';

@Component({
  selector: 'borjomi-login',
  templateUrl: 'login.component.html',
  styleUrls: ['login.component.css']
})

export class LoginComponent implements OnInit, OnDestroy {  
  authForm: FormGroup;
  isSubmitted  =  false;
  wsSubscription: Subscription;

  constructor(
    private authService: AuthService, 
    private wsService: WebSocketService,
    private router: Router) { }  

  ngOnInit(): void {
    this.initIoConnection(); 
    this.authForm  =  new FormGroup({
      usernameFormControl: new FormControl('', Validators.required),
      passwordFormControl: new FormControl('', Validators.required)
    });           
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
        () =>  console.log( 'The observable stream is complete')
      );
    
  }
  onSocketMessage(evt) {
    let obj = JSON.parse(evt);
    if (!obj) return;
    if(obj.token){
      this.authService.signIn(obj.token);
      this.router.navigateByUrl('/home');
    }    
  }

  get formControls() { return this.authForm.controls; }

  login(){
    this.isSubmitted = true;
    if(this.authForm.invalid){
      return;
    } 
    let message = {
      function: Func.LOGIN,          
      username: this.authForm.get('usernameFormControl').value,
      password: this.authForm.get('passwordFormControl').value
    };
    let lmsg = this.wsService.sendMessage(JSON.stringify(message));
    console.log(lmsg);
  }  

  closeSocket(){
    this.wsSubscription.unsubscribe();
  }

  ngOnDestroy() {
    this.closeSocket();  
  }
  
}
