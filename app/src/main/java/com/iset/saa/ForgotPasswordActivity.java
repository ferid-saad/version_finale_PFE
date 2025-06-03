package com.iset.saa;


import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;

public class ForgotPasswordActivity extends AppCompatActivity {
    //declaration des variables
    private Button BackToSignIn;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_forgot_password);
        //affectation des variables
        BackToSignIn = findViewById(R.id.back_to_sign_in);

        //actions
        BackToSignIn.setOnClickListener(v -> {

            startActivity(new Intent(this, SignInActivity.class));
        });

    }

    //declaration des méthodes
}
