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

        torch = new Torch();
	torch.setContext(this);
	tv = new TextView(this);
        tv.setText("Torch Created");
        setContentView(tv);
    }

    private class TorchTask extends Torch.EvalAssetFileTask {
	public TorchTask()
	    {
		torch.super();
	    }

	protected void onPostExecute(String result) {
	    Log.d("onPostExecute, Torch returned: %s)\n", result);
	    tv.setText(result);
	    // setContentView(tv);
	}
    };
   
    @Override
    public void onStart()  {
	super.onStart();
	Log.d("torchdemo","onStart\n");
	new TorchTask().execute("main.lua");
    }

    TextView tv;
    Torch torch;
}
