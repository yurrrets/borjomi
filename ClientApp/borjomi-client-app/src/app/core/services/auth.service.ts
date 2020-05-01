import { Injectable } from '@angular/core';
import { IUser } from '../model/user';

@Injectable({
  providedIn: 'root'
})
export class AuthService {  
    constructor() { }  
    
    public signIn(token: string){
        localStorage.setItem('access_token', token);
    }  
  
    public isLoggedIn(){
        return localStorage.getItem('access_token') !== null;  
    }  
    
    public logout(){
        localStorage.removeItem('access_token');
    }
}