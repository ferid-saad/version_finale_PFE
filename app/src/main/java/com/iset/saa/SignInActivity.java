package com.iset.saa;
import android.annotation.SuppressLint;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.iset.saa.models.SystemData;
import com.iset.saa.models.User;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

public class SignInActivity extends AppCompatActivity {
    //declaration des variables
    private TextView goToSignUp;
    private TextView goToForgotPassword;
    private TextView goToDashboardFerid;
    private EditText email, password;
    private Button btnSignIn;
    private CheckBox rememberMe;
    private String emailString, passwordString;
    private FirebaseAuth firebaseAuth;
    private ProgressDialog progressDialog;

    @SuppressLint("MissingInflatedId")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sign_in);

//affectation des variables
        goToSignUp = findViewById(R.id.go_to_sign_up);
        goToForgotPassword = findViewById(R.id.go_to_forgot_password);
        email = findViewById(R.id.email_sign_in);
        password = findViewById(R.id.password_sign_in);
        rememberMe = findViewById(R.id.remember_me);
        btnSignIn = findViewById(R.id.btn_sign_in);
        goToDashboardFerid = findViewById(R.id.btn_sign_in);

        firebaseAuth = FirebaseAuth.getInstance();
        progressDialog = new ProgressDialog(this);
/*
        FirebaseDatabase firebaseDatabase = FirebaseDatabase.getInstance();
        DatabaseReference databaseReference = firebaseDatabase.getReference("systemData");
        SystemData data = new SystemData(true, false, true, 22, 34, 55);
        databaseReference.child("" + firebaseAuth.getUid()).setValue(data);
*/
        FirebaseDatabase firebaseDatabase = FirebaseDatabase.getInstance();
        DatabaseReference databaseReference = firebaseDatabase.getReference("systemData");
        databaseReference.child("" + firebaseAuth.getUid()).get().addOnCompleteListener(new OnCompleteListener<DataSnapshot>() {
            @Override
            public void onComplete(Task<DataSnapshot> task) {
                if (!task.isSuccessful()) {
                    Log.e("firebase", "Error getting data", task.getException());
                }
                else {
                    Log.d("firebase", String.valueOf(task.getResult().getValue()));
                }
            }
        });

        //actions

        goToSignUp.setOnClickListener(v -> {

            startActivity(new Intent(SignInActivity.this, SignUpActivity.class));
        });

        goToDashboardFerid.setOnClickListener(v -> {

            startActivity(new Intent(SignInActivity.this, MainActivity.class));
        });

        goToForgotPassword.setOnClickListener(v -> {

            startActivity(new Intent(SignInActivity.this, ForgotPasswordActivity.class));
        });
        btnSignIn.setOnClickListener(v -> {
            emailString = email.getText().toString().trim();
            passwordString = password.getText().toString().trim();
            progressDialog.setMessage("Please wait...");
            progressDialog.show();

            firebaseAuth.signInWithEmailAndPassword(emailString, passwordString).addOnCompleteListener(task -> {
                if (task.isSuccessful()) {
                    checkEmailVerification();
                } else {
                    Toast.makeText(this, "Sign in failed", Toast.LENGTH_SHORT).show();
                    progressDialog.dismiss();
                }
            });

        });
    }

    private void checkEmailVerification() {
        FirebaseUser loggedUser = firebaseAuth.getCurrentUser();
        if (loggedUser != null) {
            if (loggedUser.isEmailVerified()) {
                startActivity(new Intent(this, MainActivity.class));
                finish();
            } else {
                loggedUser.sendEmailVerification();
                Toast.makeText(this, "Please Verify Email", Toast.LENGTH_SHORT).show();
                progressDialog.dismiss();
                firebaseAuth.signOut();
            }
        }
    }
    //declaration des méthodes
}
