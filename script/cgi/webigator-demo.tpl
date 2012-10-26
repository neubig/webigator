<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="ja-JP" lang="ja-JP">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>webigator デモ</title>
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
    <h1><a href="webigator-demo.cgi">webigator</a><span class="demo">デモ</span></h1>
    <!-- end .header --></div>
    
    <div class="navi">■ Home</div>
    
    <div class="memo memo_init">
      <div class="memo_tag"><a href="#" onclick="return toggleMemo()">説明</a></div>
      <div class="memo_text radius_top radius_bottom">
        これはWebからの情報集約を手伝うシステム、<a href="http://www.github.com/neubig/webigator" target="_blank">webigator</a>のデモです。<br /><br />
        各集約タスクは１種類の情報を対象としています。
        例えば、「奈良県のイベント情報」のタスクなら、システムが奈良県のイベントに関するテキスト（主にTwitter上のツイート）を提示してくれて、これを手作業で整理し、Webサイト上の一覧を作ることができます。<br /><br />
        ・<b>新しいタスクを作りたい</b>場合は「タスクの作成」ボタンをクリックしてください。<br />
        ・<b>タスク一覧に載っているタスクに参加したい</b>場合は「参加」ボタンをクリックしてください。
        <br /><br />[ <a href="#" onClick="return toggleMemo()">説明を閉じる</a> ]
      </div>
    </div>
    
  <div class="content">

    <h2>現在進行中のタスク</h2>

<TMPL_IF error>

    <p class="error"><TMPL_VAR error ESCAPE=html></p>

<TMPL_ELSE>

    <input type="button" value="タスクの作成" class="btn add" onClick="location.href='webigator-task.cgi'" />

    <div class="section">

      以下は現在進行中のタスクです。タスクに参加したい場合は「参加」ボタンをクリックしてください。<br />
      鍵マークのついたタスクへ参加するにはパスワードが必要です。

      <div class="list">
        <table width="100%">
          <tr>
            <th width="100">タスクID</th>
            <th>キーワード</th>
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
            <input type="submit" name="join_task" value="参加" class="btn<TMPL_IF NAME=has_pass> lock</TMPL_IF>" rel="#sanka" />
            </form>
            <form action="webigator-task.cgi" method="post">
            <input type="hidden" name="task_id" value="<TMPL_VAR NAME=task_id>" />
            <input type="button" value="設定" class="btn lock" rel="#settei" />
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
      <p>選択したタスクに参加するためには作業用パスワードが必要です。作業用パスワードが分からない場合はタスクの作成者にお聞きください。</p>
      <form action="webigator-run.cgi" method="post">
      <input type="hidden" name="task_id" />
      <p>パスワード <input type="password" name="pass" /></p>
      <div class="buttons"><input type="submit" class="btn close" value="OK" />&nbsp;<input type="button" class="btn close" value="キャンセル" /></div>
      </form>
    </div>

    <!-- settei dialog -->
    <div class="modal" id="settei">
      <p>設定を変更するには管理用パスワードが必要です。</p>
      <form action="webigator-task.cgi" method="post">
      <input type="hidden" name="task_id" />
      <p>パスワード <input type="password" name="pass" /></p>
      <div class="buttons"><input type="submit" name="edit_task" class="btn close" value="OK" />&nbsp;<input type="button" class="btn close" value="キャンセル" /></div>
      </form>
    </div>

    <!-- end .content --></div>
  <div class="footer">
    <!-- end .footer --></div>
  <!-- end .container --></div>
</body>
</html>
