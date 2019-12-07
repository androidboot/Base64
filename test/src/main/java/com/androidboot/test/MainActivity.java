package com.androidboot.test;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;

import com.androidboot.base64.AndroidBase64;

public class MainActivity extends AppCompatActivity {

    ImageView imageView,imageView2;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView=findViewById(R.id.imageView);
        imageView2=findViewById(R.id.imageView2);

        Bitmap bitmap = ((BitmapDrawable) imageView.getBackground()).getBitmap();

        String base64= AndroidBase64.bitmapToBase64(bitmap);
        Log.e("androidboot",base64);
        imageView2.setImageBitmap(AndroidBase64.base64ToBitmap(base64));
        //imageView2.setImageBitmap(bitmap);

    }
}
