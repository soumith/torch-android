package com.torchandroid.facedemo;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {
	
	public static int width, height;
	static TextView infoText;

	Button button768, button480, button1080;
	   
	@Override
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    
	    setContentView(R.layout.activity_main);	   

	    button768 = (Button) findViewById(R.id.button768);
	    button480 = (Button) findViewById(R.id.button480);
	    button1080 = (Button) findViewById(R.id.button240);
	    infoText = (TextView) this.findViewById(R.id.info);
	    
	    infoText.setText("This network requires 183.4 MOPs.\nIt has 3 layers like so -\n\nSpatial " +
	    		"Convolution\nSpatial Convolution Map\nSpatial Classifier");
	    
	    button768.setOnClickListener(buttonHandler);
	    button480.setOnClickListener(buttonHandler);
	    button1080.setOnClickListener(buttonHandler);
	}
	
	View.OnClickListener buttonHandler = new View.OnClickListener() 
	{
		@Override
		public void onClick(View v) 
		{
			switch(v.getId())
			{
			case R.id.button768:
				width = 1280;
				height = 768;
				break;
				
			case R.id.button480:
				width = 640;
				height = 480;
				break;
				
			case R.id.button240:
				width = 320;
				height = 240;
				break;
				
			}

			Intent startCameraActivity = new Intent(MainActivity.this, CameraActivity.class);
			MainActivity.this.startActivity(startCameraActivity);
			finish();
		}
	};
}
