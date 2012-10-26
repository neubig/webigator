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
<script type="text/javascript">
// onload
$(function(){

  /*** tweet ***/
  var keywordList = [];
  $('.keyword li').each(function(){ keywordList.push($(this).text()); });

  if (keywordList.length) {
    var keywordReg = new RegExp('(' + keywordList.join('|') + ')', 'g');
    $('.tl_td_tweet span').each(function(){
      $(this).html($(this).html().replace(keywordReg, '<span class="kw">$1</span>'));
    });
  }
  $('.kw').addClass('highlight');  // off: $('.kw').removeClass('highlight')

  /*** label event ***/
  $('.label_y_off, .label_n_off').click(function(){

    $(this).find('a').attr('id').match(/^image_([y|n])_(\d+)/);  // _(:label)_(:index)
    var label = RegExp.$1;
    var index = RegExp.$2;
    var value = $('#lab' + index).val();

    // change value
    value =  (value != label) ? label : '?';
    $('#lab' + index).val(value);

    // change class
    $('#image_y_' + index).parent().attr('class', 'label_y_off');
    $('#image_n_' + index).parent().attr('class', 'label_n_off');
    $(this).attr('class', 'label_' + label + '_' + ((value != '?') ? 'on' : 'off'));

    // change background-color
    $(this).parents('.tl_tr').find('td')
      .removeClass('bgcolor').removeClass('bgcolor_y').removeClass('bgcolor_n')
      .addClass('bgcolor' + ((value != '?') ? ('_' + value) : ''));

    return false;
  });

});
</script>
</head>

<body>

<div class="container">
  <div class="header">
    <h1><a href="webigator-demo.cgi">webigator</a><span class="demo">デモ</span></h1>
    <!-- end .header --></div>
    
    <div class="navi">■ <a href="webigator-demo.cgi">Home</a> ≫ ツイートの発見</div>
    
    <div class="memo">
      <div class="memo_tag"><a href="#" onClick="return toggleMemo()">説明</a></div>
      <div class="memo_text radius_top radius_bottom">
        有用なツイートを発見します。<br /><br />
        
        注：現在は最初の方に有用でないツイートばかりがでます。この問題は改善予定ですが、今は有用なものが一個でも出るまで我慢すれば問題が改善されます。
        <br /><br />[ <a href="#" onClick="return toggleMemo()">説明を閉じる</a> ]
      </div>
    </div>
    
  <div class="content">

    <h2>ツイートの発見</h2>

<TMPL_IF error>

    <p class="error"><TMPL_VAR error ESCAPE=html></p>
    <input type="button" value="タスク一覧に戻る" class="btn" onClick="location.href='webigator-demo.cgi'" />

<TMPL_ELSE>

    <div class="keyword">
      <div class="keyword_lst radius_top radius_bottom">
        <table><tr><th>現在のキーワード：</th><td><ul>
  <TMPL_IF keywords>
    <TMPL_LOOP NAME=keywords>
          <li><TMPL_VAR NAME=keyword ESCAPE=html></li>
    </TMPL_LOOP>
  <TMPL_ELSE>
          <li>(未登録)</li>
  </TMPL_IF>
        </ul><br class="clearfloat" /></td></tr></table>
      </div>
    </div>

  <TMPL_IF list>

    <div class="section">

      有用なものは"＋"のラベルを、有用でないものは"－"のラベルを選択してください。<br />
      判断のつかないものは、そのままでかまいません。投稿後、新しいツイートが表示されます。<br />

      <div class="tweet list">
        <form action="webigator-run.cgi" method="post">
        <input type="hidden" name="task_id" value="<TMPL_VAR task_id>">
        <table width="100%">
          <tr>
            <th class="tl_td_label">ラベル</th>
            <th>テキスト</th>
            <th>ツイートID</th>
          </tr>

    <TMPL_LOOP NAME=list>
          <tr class="tl_tr">
            <td class="tl_td_label">
              <ul>
                <li class="label_y_off"><a href="#" id="image_y_<TMPL_VAR NAME=index>"></a></li>
                <li class="label_n_off"><a href="#" id="image_n_<TMPL_VAR NAME=index>"></a></li>
              </ul>
              <input type="hidden" id="lab<TMPL_VAR NAME=index>" name="lab<TMPL_VAR NAME=index>" value="?" />
            </td>
            <td class="tl_td_tweet">
              <span><TMPL_VAR NAME=text ESCAPE=html></span>
              <input type="hidden" name="text<TMPL_VAR NAME=index>" value="<TMPL_VAR NAME=text ESCAPE=html>" />
            </td>
            <td><TMPL_VAR NAME=id><input type="hidden" name="id<TMPL_VAR NAME=index>" value="<TMPL_VAR NAME=id>" /></td>
          </tr>
    </TMPL_LOOP>

        </table>
        <input type="submit" name="post_labels" value="ラベルを投稿" class="btn" />
        </form>
      </div>
    </div>

  <TMPL_ELSE>

    <br />
    <p class="error">現在、新しく閲覧できるデータがありません。このキーワードに関して新たに取得できるテキストがなかったか、情報を取得するプログラムが動いていない可能性があります。</p>

    <form action="webigator-run.cgi" method="post">
    <input type="hidden" name="task_id" value="<TMPL_VAR task_id>">
    <input type="submit" value="再取得" class="btn" />
    </form>

  </TMPL_IF>

</TMPL_IF>

    <!-- end .content --></div>
  <div class="footer">
    <!-- end .footer --></div>
  <!-- end .container --></div>
</body>
</html>