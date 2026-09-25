package com.revc;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.File;

public class LauncherActivity extends AppCompatActivity {

    private static final int PERMISSION_REQUEST_CODE = 1001;
    private static final int MANAGE_STORAGE_REQUEST_CODE = 1002;
    private String gameFilesPath;

    static {
        System.loadLibrary("revc");
    }

    // Método nativo para establecer la ruta de archivos del juego
    public native void setenv(String path);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Determinar ruta de archivos del juego
        gameFilesPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/reVC";

        // Crear directorio si no existe
        File gameDir = new File(gameFilesPath);
        if (!gameDir.exists()) {
            gameDir.mkdirs();
        }

        // Auto-launch si ya hay permisos y archivos
        if (hasPermissions() && hasGameFiles()) {
            launchGameDirectly();
            return;
        }

        setContentView(R.layout.activity_launcher);
        
        TextView pathText = findViewById(R.id.pathText);
        pathText.setText("Ruta de archivos: " + gameFilesPath);

        Button launchButton = findViewById(R.id.launchButton);
        launchButton.setOnClickListener(v -> checkPermissionsAndLaunch());
    }

    private boolean hasPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return Environment.isExternalStorageManager();
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    == PackageManager.PERMISSION_GRANTED;
        }
        return true;
    }

    private boolean hasGameFiles() {
        File gta3Img = new File(gameFilesPath + "/models/gta3.img");
        return gta3Img.exists();
    }

    private void launchGameDirectly() {
        setenv(gameFilesPath);
        Intent intent = new Intent(this, GameActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startActivity(intent);
        finish();
    }

    private void checkPermissionsAndLaunch() {
        // Android 11+ (API 30+)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                Toast.makeText(this, "Se requiere acceso a todos los archivos", Toast.LENGTH_LONG).show();
                try {
                    Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                    intent.setData(Uri.parse("package:" + getPackageName()));
                    startActivityForResult(intent, MANAGE_STORAGE_REQUEST_CODE);
                } catch (Exception e) {
                    Intent intent = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                    startActivityForResult(intent, MANAGE_STORAGE_REQUEST_CODE);
                }
                return;
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{
                                Manifest.permission.READ_EXTERNAL_STORAGE,
                                Manifest.permission.WRITE_EXTERNAL_STORAGE
                        },
                        PERMISSION_REQUEST_CODE);
                return;
            }
        }
        launchGame();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == MANAGE_STORAGE_REQUEST_CODE) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                if (Environment.isExternalStorageManager()) {
                    launchGame();
                } else {
                    Toast.makeText(this, "Permiso denegado. La app necesita acceso a archivos.", Toast.LENGTH_SHORT).show();
                }
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                launchGame();
            } else {
                Toast.makeText(this, "Permisos de almacenamiento requeridos", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void launchGame() {
        // Verificar que existan los archivos del juego
        File gta3Img = new File(gameFilesPath + "/models/gta3.img");
        if (!gta3Img.exists()) {
            Toast.makeText(this, 
                "Copia los archivos del juego a: " + gameFilesPath, 
                Toast.LENGTH_LONG).show();
            return;
        }

        // Establecer variable de entorno para los archivos del juego
        setenv(gameFilesPath);

        // Iniciar actividad del juego
        Intent intent = new Intent(this, GameActivity.class);
        startActivity(intent);
    }
}
