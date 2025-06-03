package com.iset.saa;


import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.iset.saa.models.User;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class SignUpActivity extends AppCompatActivity {
    //declaration des variables
    private TextView goToSignIn;
    private EditText userName, email, cin, phoneNumber, password, confirmPassword;
    private Button btnSignUP;
    private String userNameString, emailString, cinString, phoneNumberString, passwordString, confirmPasswordString;
    private static final String EMAIL_REGEX = "^[A-Za-z0-9+_.-]+@(.+)$";
    private FirebaseAuth firebaseAuth;
    private ProgressDialog progressDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sign_up);
//affectation des variables
        goToSignIn = findViewById(R.id.go_to_sign_in);
        userName = findViewById(R.id.user_name_sign_up);
        email = findViewById(R.id.email_sign_up);
        cin = findViewById(R.id.cin_sign_up);
        phoneNumber = findViewById(R.id.phone_number_sign_up);
        password = findViewById(R.id.password_sign_up);
        confirmPassword = findViewById(R.id.confirm_password_sign_up);
        btnSignUP = findViewById(R.id.btn_sign_up);
        firebaseAuth = FirebaseAuth.getInstance();
        progressDialog = new ProgressDialog(this);

        //actions
        btnSignUP.setOnClickListener(v -> {
            if (validate()) {
                progressDialog.setMessage("Please wait...");
                progressDialog.show();

                firebaseAuth.createUserWithEmailAndPassword(emailString, passwordString).addOnCompleteListener(task -> {
                    if (task.isSuccessful()) {

                        sendEmailVerification();
                    } else {
                        Exception e = task.getException();
                        Toast.makeText(this, "Failed: " + (e != null ? e.getMessage() : "Unknown error"), Toast.LENGTH_LONG).show();
                        progressDialog.dismiss();
                    }
                });

            }
        });
        goToSignIn.setOnClickListener(v -> {

            startActivity(new Intent(SignUpActivity.this, SignInActivity.class));
        });

    }

    //declaration des méthodes
    private void sendEmailVerification() {
        FirebaseUser loggedUser = firebaseAuth.getCurrentUser();
        if (loggedUser != null) {
            loggedUser.sendEmailVerification().addOnCompleteListener(task -> {
                if (task.isSuccessful()) {
                    sendUserData();
                    Toast.makeText(this, "Registration done please check your email", Toast.LENGTH_LONG).show();
                    startActivity(new Intent(this, SignInActivity.class));
                    progressDialog.dismiss();
                    finish();
                } else {
                    Toast.makeText(this, "Failed", Toast.LENGTH_SHORT).show();
                    progressDialog.dismiss();

                }
            });
        }
    }

    private void sendUserData() {
        FirebaseDatabase firebaseDatabase = FirebaseDatabase.getInstance();
        DatabaseReference databaseReference = firebaseDatabase.getReference("users");
        User user = new User(userNameString, emailString, cinString, phoneNumberString);
        databaseReference.child("" + firebaseAuth.getUid()).setValue(user);
    }

    private boolean validate() {
        boolean result = false;
        userNameString = userName.getText().toString().trim();
        emailString = email.getText().toString().trim();
        cinString = cin.getText().toString().trim();
        phoneNumberString = phoneNumber.getText().toString().trim();
        passwordString = password.getText().toString().trim();
        confirmPasswordString = confirmPassword.getText().toString().trim();
        if (userNameString.length() < 7) {
            userName.setError("User name is invalid");
        } else if (!isValidEmail(emailString)) {
            email.setError("Email is invalid");
        } else if (cinString.length() != 8) {
            cin.setError("CIN is invalid");
        } else if (phoneNumberString.length() != 8) {
            phoneNumber.setError("Phone number is invalid");
        } else if (passwordString.length() < 6) {
            password.setError("Password is invalid");
        } else if (!confirmPasswordString.equals(passwordString)) {
            confirmPassword.setError(" confirm password is invalid");
        } else {
            result = true;
        }
        return result;
    }


    private boolean isValidEmail(String email) {
        Pattern pattern = Pattern.compile(EMAIL_REGEX);
        Matcher matcher = pattern.matcher(email);
        return matcher.matches();
    }
}