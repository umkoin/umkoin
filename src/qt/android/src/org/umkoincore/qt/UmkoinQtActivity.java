package org.umkoincore.qt;

import android.os.Bundle;
import android.system.ErrnoException;
import android.system.Os;

import org.qtproject.qt5.android.bindings.QtActivity;

import java.io.File;

public class UmkoinQtActivity extends QtActivity
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        final File umkoinDir = new File(getFilesDir().getAbsolutePath() + "/.umkoin");
        if (!umkoinDir.exists()) {
            umkoinDir.mkdir();
        }

        super.onCreate(savedInstanceState);
    }
}
