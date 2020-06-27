import { Component, OnInit, OnDestroy } from '@angular/core';
import { WebSocketService } from '../core/services/websocket.service';
import { Router } from '@angular/router';
import { Subscription } from 'rxjs';
import { FormControl, Validators, FormGroup, FormBuilder } from '@angular/forms';
import { IAccount } from '../core/model/account';

@Component({
    selector: 'borjomi-home',
    templateUrl: 'home.component.html',
    styleUrls: []
  })
  
export class HomeComponent implements OnInit {
  dataSubscription: Subscription;  
  childAccounts: IAccount[];
  currentAccount: IAccount;
  currentAccountName: string;
  //accountForm : FormGroup;
  //accountControl = new FormControl();   
  accountForm = this.fb.group({
    accountControl: [null, Validators.required]
  })
   
    
  constructor(private wsService: WebSocketService,
      public fb: FormBuilder,
      private router: Router ) {}  
  
    
  ngOnInit(): void {
    if (this.dataSubscription) {
      this.dataSubscription.unsubscribe();
    }
        

    this.wsService.getChildAccounts()
      .subscribe(data => {
        console.log(data);
        this.childAccounts = data.childAccounts;
        this.currentAccount = data.childAccounts[0];
        //this.accountId.setValue(data.childAccounts[0].id, {
          //onlySelf: true
        //});
        this.currentAccountName = data.childAccounts[0].username;
        //console.log(this.currentAccountName);
        localStorage.setItem('childAccounts', data.childAccounts);
      },
      err => console.log(err),
      () =>  console.log( 'The observable stream is complete'))
    ;   
    
  } 

  //get formControls() { return this.accountForm.controls; }

  get account() {
    return this.accountForm.get('accountControl');
  }
    
  changeAccount() {   
    console.log(this.account.value);    
    this.currentAccount = this.account.value;
    this.currentAccountName = this.currentAccount.username;
  }
  
   
}