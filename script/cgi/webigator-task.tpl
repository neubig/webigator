<!doctype html>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="ja-JP" lang="ja-JP">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>webigator デモ</title>
<link href="<TMPL_VAR top_dir>/css/main.css" rel="stylesheet" type="text/css" />
<link href="<TMPL_VAR top_dir>/css/webigator.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
<script type="text/javascript" src="<TMPL_VAR top_dir>/js/jquery.cookie.js"></script>
<script type="text/javascript" src="<TMPL_VAR top_dir>/js/webigator.js"></script>
</head>

<body>

<div class="container">
  <div class="header">
    <h1><a href="webigator-demo.cgi">webigator</a><span class="demo">デモ</span></h1>
    <!-- end .header --></div>
    
    <div class="navi">■ <a href="webigator-demo.cgi">Home</a> ≫ タスクの作成</div>
    
    <div class="memo">
      <div class="memo_tag"><a href="#" onClick="return toggleMemo()">説明</a></div>
      <div class="memo_text radius_top radius_bottom">
        タスクを作成します。
        <br /><br />[ <a href="#" onClick="return toggleMemo()">説明を閉じる</a> ]
      </div>
    </div>
    
  <div class="content">
    
    <h2>タスクの作成</h2>
    
<TMPL_IF error>

    <p class="error"><TMPL_VAR error ESCAPE=html></p>
    <input type="button" value="タスク一覧に戻る" class="btn" onClick="location.href='webigator-demo.cgi'" />

<TMPL_ELSE>

  <TMPL_IF success>
    
    <p>新しいタスクを作成しました。</p>
    <form action="webigator-run.cgi" method="post">
      <input type="hidden" name="task_id" value="<TMPL_VAR task_id ESCAPE=html>" />
      <input type="hidden" name="user_pass" value="<TMPL_VAR user_pass ESCAPE=html>" />
      <input type="submit" value="タスクに参加する" class="btn" />
      <input type="button" value="タスク一覧に戻る" class="btn" onClick="location.href='webigator-demo.cgi'" />
    </form>

  <TMPL_ELSE>

    <p class="error"><TMPL_VAR alert ESCAPE=html></p>
    
    必要な項目を入力し、「登録」ボタンをクリックしてください。
    
    <form action="webigator-task.cgi" method="post">
    <input type="hidden" name="task_id" value="<TMPL_VAR task_id ESCAPE=html>">
    <div class="form">
    <table width="100%">
      <tr>
        <th>検索キーワード (必須)</th><td><input type="text" name="task_keywords" value="<TMPL_VAR task_keywords ESCAPE=html>" style="width:400px;" /><br />複数のキーワードを指定する場合は、空白で区切ってください。</td>
      </tr>
      <tr>
        <th>管理用パスワード (必須)</th><td><input type="text" name="admin_pass" value="<TMPL_VAR admin_pass ESCAPE=html>" /><br />登録したタスクの設定を変更する場合に使用します。</td>
      </tr>
      <tr>
        <th>作業用パスワード</th><td><input type="text" name="user_pass" value="<TMPL_VAR user_pass ESCAPE=html>" /><br />パスワードを設定した場合、タスク参加時にパスワードの入力を求められます。<br />パスワードを設定しない場合は、誰でもタスクに参加できます。</td>
      </tr>
    </table>
    </div>
    <div style="text-align:center;">
      <input type="submit" name="add_task" value="登録" class="btn" />
      <!--&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" value="削除" class="btn" onClick="location.href='webigator-task.cgi?del_task=1&task_id=<TMPL_VAR task_id ESCAPE=html>'" />-->
    </div>
    </form>

  </TMPL_IF>

</TMPL_IF>

    <!-- end .content --></div>
  <div class="footer">
    <!-- end .footer --></div>
  <!-- end .container --></div>
</body>
</html>