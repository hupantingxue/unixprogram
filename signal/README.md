<h3>How to build</h3>
<pre></code>
mkdir build
cd build
cmake28 ..
make
</code></pre>

<h3>How to run</h3>
send signal to signal01
-----------
kill -USR1  7632 
kill -USR2  7632  
kill -USR3  7632  


./signal01 output
-------------
received SIGUSR1 singal.
received SIGUSR2 singal.
Killed
