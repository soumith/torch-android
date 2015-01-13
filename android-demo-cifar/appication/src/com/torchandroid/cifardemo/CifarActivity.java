package com.torchandroid.cifardemo;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.torchandroid.cifardemo.lua.LuaManager;
import com.torchandroid.cifardemo.util.Util;

public class CifarActivity extends Activity {

	final private String TAG = "CifarActivity";
	private Button mResourceRecognitionTestButton;
	private Button mTestDataSetTestButton;
	private LinearLayout mMainLayout;
	private LuaManager mLuaManager;
	private LayoutInflater mInflater;

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		requestWindowFeature(Window.FEATURE_NO_TITLE);

		setContentView(R.layout.activity_cifar);
		mMainLayout = (LinearLayout) findViewById(R.id.main_layout);
		mLuaManager = LuaManager.getLuaManager(this);
		mResourceRecognitionTestButton = (Button) findViewById(R.id.recog_btn);
		mTestDataSetTestButton = (Button) findViewById(R.id.test_btn);
		mInflater = LayoutInflater.from(this);
		mResourceRecognitionTestButton
				.setOnClickListener(new OnClickListener() {

					@Override
					public void onClick(View v) {
						startSampleRecognition();
					}
				});

		mTestDataSetTestButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				long startTime = System.nanoTime();
				float result = mLuaManager.testTorchData();
				float delay = (float) (System.nanoTime() - startTime) / 1000000000.0f;
				mTestDataSetTestButton
						.setText("Result:\nCorrect Recognition Rate : "
								+ result + "\nDelay : " + delay
								+ "sec for 1000 data");
			}
		});
		for (int i = 0; i < 10; i++)
			addTestset(i);
	}

	private void addTestset(int index) {
		LinearLayout linearLayout = (LinearLayout) mInflater.inflate(
				R.layout.cifar_image_category, mMainLayout, false);
		LinearLayout imageViewArea = (LinearLayout) linearLayout
				.findViewById(R.id.image_view_area);
		TextView categoryTitle = (TextView) linearLayout
				.findViewById(R.id.category_title);
		categoryTitle.setText(Util.classes[index]);
		// for showing what images will be tested
		for (int i = 0; i < 10; i++) {
			ImageView imageView = new ImageView(this);
			imageView.setLayoutParams(new LayoutParams(64, 64));
			imageView.setImageResource(Util.imageDrawableIds[index][i]);
			imageViewArea.addView(imageView);
		}

		Button button = (Button) linearLayout.findViewById(R.id.category_test);
		button.setText("Test " + Util.classes[index]);
		button.setTag(index);
		button.setOnClickListener(mCategoryTestClickListener);
		mMainLayout.addView(linearLayout);
	}

	OnClickListener mCategoryTestClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {
			int index = (Integer) v.getTag();
			startRecognition(index, (Button) v);
		}
	};

	private void startSampleRecognition() {
		int correct = 0, wrong = 0;
		String resultString = "Result :";
		long startTime = System.nanoTime();
		for (int index = 0; index < Util.imageDrawableIds.length; index++) {

			for (int i = 0; i < Util.imageDrawableIds[index].length; i++) {
				int result = mLuaManager.getTopRecognitionResult(32, 32, Util
						.getImageRGBA(getResources(),
								Util.imageDrawableIds[index][i]));
				if (result == index + 1) {
					correct++;
				} else {
					wrong++;
				}
			}
		}
		float delay = (float) (System.nanoTime() - startTime) / 1000000000.0f;
		float rate = (float) correct / (correct + wrong);
		resultString = resultString + "\n Correct Recognition Rate :" + rate
				+ "\nDelay : " + delay + "sec for " + (correct + wrong)
				+ " data";
		Log.d(TAG, resultString);
		mResourceRecognitionTestButton.setText(resultString);
	}

	private void startRecognition(int index, Button button) {
		int correct = 0, wrong = 0;
		for (int i = 0; i < Util.imageDrawableIds[index].length; i++) {
			int result = mLuaManager.getTopRecognitionResult(32, 32, Util
					.getImageRGBA(getResources(),
							Util.imageDrawableIds[index][i]));
			if (result == index + 1) {
				correct++;
			} else {
				wrong++;
			}
		}
		float rate = (float) correct / (correct + wrong);
		Log.d(TAG, Util.classes[index] + " correct rate : " + rate);
		button.setText(Util.classes[index] + " correct rate : " + rate);
	}
}