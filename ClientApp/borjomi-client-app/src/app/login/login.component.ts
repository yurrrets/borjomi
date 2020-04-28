import { Component, OnInit, Inject } from '@angular/core';
import { FormControl, Validators } from '@angular/forms';
import { MatDialogRef, MAT_DIALOG_DATA } from '@angular/material/dialog';

@Component({
  selector: 'app-login',
  templateUrl: 'login.component.html',
  styleUrls: ['login.component.css']
})

export class LoginComponent implements OnInit {
  usernameFormControl = new FormControl('', [Validators.required]);
  passwordFormControl = new FormControl('', [Validators.required]);
  previousUsername: string;
  
  constructor(public dialogRef: MatDialogRef<LoginComponent>,
    @Inject(MAT_DIALOG_DATA) public params: any) { 
      this.previousUsername = params.username ? params.username : undefined;
      this.usernameFormControl.setValue(params.username);
      this.passwordFormControl.setValue(params.password);
    }

  ngOnInit(): void {
  }

  public login(): void {
    this.dialogRef.close({
      username: this.usernameFormControl.value,
      password: this.passwordFormControl.value
    });
    //pass data to server
  }

}
