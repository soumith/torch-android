package com.torchandroid.cifardemo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity {

	private Button mButtonCifar;
	private Button mButtonImageCifar;


	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		mButtonCifar = (Button) findViewById(R.id.button_cifar);
		mButtonCifar.setOnClickListener(mCifarbuttonClickListener);

		mButtonImageCifar = (Button) findViewById(R.id.button_image_cifar);
		mButtonImageCifar.setOnClickListener(mCifarImagebuttonClickListener);
	}

	View.OnClickListener mCifarbuttonClickListener = new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			Intent startCifarActivity = new Intent(MainActivity.this,
					CifarActivity.class);
			startActivity(startCifarActivity);
		}
	};

	View.OnClickListener mCifarImagebuttonClickListener = new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			Intent startCifarImageActivity = new Intent(MainActivity.this,
					CifarImageActivity.class);
			startActivity(startCifarImageActivity);
		}
	};
}
