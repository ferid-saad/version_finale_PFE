package com.iset.saa;

import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {  //classe principale de l’application, elle hérite de AppCompatActivity

    DatabaseReference dashDataRef;

    @Override
    protected void onCreate(Bundle savedInstanceState) { //Méthode appelée lors de la création de l'activité
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);//chargement de fichier activity_main.xml(interface utilisateur

//Initialisation des éléments d'interface
        TextView temperature = findViewById(R.id.temperature);
        TextView humidite = findViewById(R.id.humidite);
        TextView humiditeSol = findViewById(R.id.humiditeSol);
        TextView temperatureExt = findViewById(R.id.temperatureExt);
        TextView humiditeExt = findViewById(R.id.humiditeExt);
        TextView windExt = findViewById(R.id.windExt);


        ToggleButton motopompe = findViewById(R.id.motopompe);
        ToggleButton vanne1 = findViewById(R.id.vanne1);
        ToggleButton vanne2 = findViewById(R.id.vanne2);

//Connexion à Firebase à la racine "data"

        dashDataRef = FirebaseDatabase.getInstance().getReference("data");

        //Empêche de démarrer la motopompe si les deux vannes sont fermées
        motopompe.setOnClickListener(v -> {
            if(!motopompe.isChecked() && vanne1.isChecked() && vanne2.isChecked()) {
                Toast toast = Toast.makeText(getApplicationContext(), "Veuillez ouvrir au moins une vanne pour pouvoir démarrer la motopompe!", Toast.LENGTH_LONG);
                toast.setGravity(Gravity.CENTER, 0, 0);
                toast.show();
                motopompe.setChecked(true);
                return;
            }
            dashDataRef.child("motopompe").setValue(!motopompe.isChecked());
        });
        //Empêche de fermer les deux vannes si la motopompe est en marche
        vanne1.setOnClickListener(v -> {
            if(!motopompe.isChecked() && vanne1.isChecked() && vanne2.isChecked()) {
                Toast toast = Toast.makeText(getApplicationContext(), "Vous ne pouvez pas fermer les deux vannes lorsque la motopompe est en marche!", Toast.LENGTH_LONG);
                toast.setGravity(Gravity.CENTER, 0, 0);
                toast.show();
                vanne1.setChecked(false);
                return;
            }
            dashDataRef.child("vanne1").setValue(!vanne1.isChecked());
        });

        vanne2.setOnClickListener(v -> {
            if(!motopompe.isChecked() && vanne1.isChecked() && vanne2.isChecked()) {
                Toast toast = Toast.makeText(getApplicationContext(), "Vous ne pouvez pas fermer les deux vannes lorsque le motopompe est en marche!", Toast.LENGTH_LONG);
                toast.setGravity(Gravity.CENTER, 0, 0);
                toast.show();
                vanne2.setChecked(false);
                return;
            }
            dashDataRef.child("vanne2").setValue(!vanne2.isChecked());
        });

        dashDataRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
//Cette méthode écoute en permanence les changements dans Firebase sous "data"
                DashData data = dataSnapshot.getValue(DashData.class);

                //Convertit les données récupérées en objet DashData

                temperature.setText(String.valueOf(data.temperature) + "°");
                humidite.setText(String.valueOf(data.humidite) + "%");
                humiditeSol.setText(String.valueOf(data.humiditeSol) + "%");
                temperatureExt.setText(String.valueOf(data.temperatureExt) + "°");
                humiditeExt.setText(String.valueOf(data.humiditeExt) + "%");
                windExt.setText(String.valueOf(data.vitesseVent) + "km/h");


                motopompe.setChecked(Boolean.TRUE.equals(!data.motopompe));
                vanne1.setChecked(Boolean.TRUE.equals(!data.vanne1));
                vanne2.setChecked(Boolean.TRUE.equals(!data.vanne2));
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {
                Log.w("firebase", "error", error.toException());
            }

        });

    }

}