<!doctype html>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="ja-JP" lang="ja-JP">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>webigator Demo</title>
<link href="<TMPL_VAR top_dir>/css/main.css" rel="stylesheet" type="text/css" />
<link href="<TMPL_VAR top_dir>/css/webigator.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
<script type="text/javascript" src="<TMPL_VAR top_dir>/js/jquery.cookie.js"></script>
<script type="text/javascript" src="<TMPL_VAR top_dir>/js/webigator.js"></script>
</head>

<body>

<div class="container">
  <div class="header">
    <h1><a href="webigator-demo.cgi">webigator</a><span class="demo"> Demo</span></h1>
    <!-- end .header --></div>
    
    <div class="navi">&gt; <a href="webigator-demo.cgi">Home</a> &gt;&gt; Create Task</div>
    
    <div class="memo">
      <div class="memo_tag"><a href="#" onClick="return toggleMemo()">Explanation</a></div>
      <div class="memo_text radius_top radius_bottom">
        On this page you can create a new information filtering task.
        <br /><br />[ <a href="#" onClick="return toggleMemo()">Close Explanation</a> ]
      </div>
    </div>
    
  <div class="content">
    
    <h2>Create a Task</h2>
    
<TMPL_IF error>

    <p class="error"><TMPL_VAR error ESCAPE=html></p>
    <input type="button" value="Return to Task List" class="btn" onClick="location.href='webigator-demo.cgi'" />

<TMPL_ELSE>

  <TMPL_IF success>
    
    <p>The new task was created!</p>
    <form action="webigator-run.cgi" method="post">
      <input type="hidden" name="task_id" value="<TMPL_VAR task_id ESCAPE=html>" />
      <input type="hidden" name="pass" value="<TMPL_VAR user_pass ESCAPE=html>" />
      <input type="submit" value="Participate" class="btn" />
      <input type="button" value="Return to Task List" class="btn" onClick="location.href='webigator-demo.cgi'" />
    </form>

  <TMPL_ELSE>

    <p class="error"><TMPL_VAR alert ESCAPE=html></p>
    
    Fill out the required fields and press "Create."
    
    <form action="webigator-task.cgi" method="post">
    <input type="hidden" name="task_id" value="<TMPL_VAR task_id ESCAPE=html>">
    <div class="form">
    <table width="100%">
      <tr>
        <th>Search Keywords<br />(Required)</th><td><input type="text" name="task_keywords" value="<TMPL_VAR task_keywords ESCAPE=html>" style="width:400px;" /><br />If you want to create multiple keywords, separate them with a space.</td>
      </tr>
      <tr>
        <th>Administrator Password<br />(Required)</th><td><input type="text" name="pass" value="<TMPL_VAR admin_pass ESCAPE=html>" /><br />This can be used when you want to change the information about a particular task.</td>
      </tr>
      <tr>
        <th>Participant Password<br />(Optional)</th><td><input type="text" name="user_pass" value="<TMPL_VAR user_pass ESCAPE=html>" /><br />
        This can be used to restrict access to the task interface.</td>
      </tr>
    </table>
    </div>
    <TMPL_IF allow_task_add>
    
      <div style="text-align:center;">
        <input type="submit" name="add_task" value="Create" class="btn" />
        <!--&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" value="Delete" class="btn" onClick="location.href='webigator-task.cgi?del_task=1&task_id=<TMPL_VAR task_id ESCAPE=html>'" />-->
      </div>
    
    <TMPL_ELSE>

      <p class="error">Currently, for demo purposes it is not possible to add new tasks.</p>
      <input type="button" value="Return to Task List" class="btn" onClick="location.href='webigator-demo.cgi'" />

    </TMPL_IF>

    </form>

  </TMPL_IF>

</TMPL_IF>

    <!-- end .content --></div>
  <div class="footer">
    <!-- end .footer --></div>
  <!-- end .container --></div>
</body>
</html>
