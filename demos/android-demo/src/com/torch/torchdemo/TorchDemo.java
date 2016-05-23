package com.torch.torchdemo;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.util.Log;

import com.torch.Torch;

public class TorchDemo extends Activity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
	Log.d("torchdemo","Hello from JAVA\n");

        torch = new Torch(this);
	tv = new TextView(this);
        tv.setText("Torch Created.");
        setContentView(tv);
    }


    @Override
    public void onStart()  {
	super.onStart();
	Log.d("torchdemo","onStart\n");
        Runnable r = new Runnable() {
                public void run() {
                    String returnFromC = torch.call("main.lua");
                    tv.setText(returnFromC);
                    //                    setContentView(tv);
                }
            };
        r.run();
    }

    TextView tv;
    Torch torch;
}
