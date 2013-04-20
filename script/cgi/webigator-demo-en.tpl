<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="ja-JP" lang="ja-JP">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>webigator Demo</title>
<link href="<TMPL_VAR top_dir>/css/main.css" rel="stylesheet" type="text/css" />
<link href="<TMPL_VAR top_dir>/css/webigator.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
<script type="text/javascript" src="http://cdn.jquerytools.org/1.2.7/full/jquery.tools.min.js"></script>
<script type="text/javascript" src="<TMPL_VAR top_dir>/js/jquery.cookie.js"></script>
<script type="text/javascript" src="<TMPL_VAR top_dir>/js/webigator.js"></script>
<script type="text/javascript">
// onload
$(function(){

  /*** lock event ***/
  $(".lock")
    .click(function(){
      var taskId = $(this).parent().parent().find('input[name=task_id]').val();
      $($(this).attr('rel')).find('input[name=task_id]').val(taskId);
    })
    .overlay({
        mask: {
          color: '#000',
          loadSpeed: 100,
          opacity: 0.4
        },
        closeOnClick: false
    });

});
</script>
</head>

<body>

<div class="container">
  <div class="header">
    <h1><a href="webigator-demo.cgi">webigator</a><span class="demo">Demo</span></h1>
    <!-- end .header --></div>
    
    <div class="navi">&gt; Home</div>
    
    <div class="memo memo_init">
      <div class="memo_tag"><a href="#" onclick="return toggleMemo()">Explanation</a></div>
      <div class="memo_text radius_top radius_bottom">
        This is a demo of <a href="http://www.github.com/neubig/webigator" target="_blank">webigator</a>, a system for aggregating information from the web.<br /><br />
        It can handle a number of tasks, each of which is designed ot aggregate one type of information.
        For example, if we fant to find information about "events in Nara prefecture," the system will display tweets or other documents that are likely to contain information about this subject.
        <br /><br />
        * <b>To create a new task</b> click the "Create Task" button.<br />
        * <b>To join in an existing task</b> click the "Join Task" button.
        <br /><br />[ <a href="#" onClick="return toggleMemo()">Close Explanation</a> ]
      </div>
    </div>
    
  <div class="content">

    <h2>Current Tasks</h2>

<TMPL_IF error>

    <p class="error"><TMPL_VAR error ESCAPE=html></p>

<TMPL_ELSE>

    <input type="button" value="Create Task" class="btn add" onClick="location.href='webigator-task.cgi'" />

    <div class="section">

      The following tasks are currently in progress. If you want to join a task click the "Join" button.<br />
      Tasks with the lock symbol require a password to enter.

      <div class="list">
        <table width="100%">
          <tr>
            <th width="100">Task ID</th>
            <th>Keyword</th>
            <th width="200">&nbsp;</th>
          </tr>

  <TMPL_LOOP NAME=LIST>
          <tr class="tl_tr">
            <!-- <form action="webigator-run.cgi" method="post"> -->
            <td><TMPL_VAR NAME=task_id></td>
            <td class="tl_td_tweet"><span><TMPL_VAR NAME=keywords ESCAPE=html></span></td>
            <td align="right" nowrap>
            <form action="webigator-run.cgi" method="post">
            <input type="hidden" name="task_id" value="<TMPL_VAR NAME=task_id>" />
            <input type="submit" name="join_task" value="Join" class="btn<TMPL_IF NAME=has_pass> lock</TMPL_IF>" rel="#sanka" />
            <input type="button" name="edit_task" value="Settings" class="btn lock" rel="#settei" />
            </form>
            </td>
            <!-- </form> -->
          </tr>
  </TMPL_LOOP>

        </table>
      </div>

    </div>

</TMPL_IF>

    <!-- sanka dialog -->
    <div class="modal" id="sanka">
      <p>In order to join this task you must have a password. Please enter the password, or ask the task administrator if you don't know it.
      <form action="webigator-run.cgi" method="post">
      <input type="hidden" name="task_id" />
      <p>Password <input type="password" name="pass" /></p>
      <div class="buttons"><input type="submit" class="btn close" value="OK" />&nbsp;<input type="button" class="btn close" value="Cancel" /></div>
      </form>
    </div>

    <!-- settei dialog -->
    <div class="modal" id="settei">
      <p>To change the task settings you must have a password.</p>
      <form action="webigator-task.cgi" method="post">
      <input type="hidden" name="task_id" />
      <p>Password <input type="password" name="pass" /></p>
      <div class="buttons"><input type="submit" name="edit_task" class="btn close" value="OK" />&nbsp;<input type="button" class="btn close" value="Cancel" /></div>
      </form>
    </div>

    <!-- end .content --></div>
  <div class="footer">
    <!-- end .footer --></div>
  <!-- end .container --></div>
</body>
</html>
