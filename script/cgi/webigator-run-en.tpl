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
    <h1><a href="webigator-demo.cgi">webigator</a><span class="demo">Demo</span></h1>
    <!-- end .header --></div>
    
    <div class="navi">&gt; <a href="webigator-demo.cgi">Home</a> &gt;&gt; Discover Information</div>
    
    <div class="memo">
      <div class="memo_tag"><a href="#" onClick="return toggleMemo()">Explanation</a></div>
      <div class="memo_text radius_top radius_bottom">
        This is an interface to find useful information.<br /><br />
        
        Note: In some cases, when you first start a task the accuracy may be very low, particularly until you find one good example. If you encounter this, just be patient and once you find a good example the accuracy should improve dramatically.
        <br /><br />[ <a href="#" onClick="return toggleMemo()">Close Explanation</a> ]
      </div>
    </div>
    
  <div class="content">

    <h2>Find Information</h2>

<TMPL_IF error>

    <p class="error"><TMPL_VAR error ESCAPE=html></p>
    <input type="button" value="Return to Task List" class="btn" onClick="location.href='webigator-demo.cgi'" />

<TMPL_ELSE>

    <div class="keyword">
      <div class="keyword_lst radius_top radius_bottom">
        <table><tr><th>Current Keywords</th><td><ul>
  <TMPL_IF keywords>
    <TMPL_LOOP NAME=keywords>
          <li><TMPL_VAR NAME=keyword ESCAPE=html></li>
    </TMPL_LOOP>
  <TMPL_ELSE>
          <li>(None Specified)</li>
  </TMPL_IF>
        </ul><br class="clearfloat" /></td></tr></table>
      </div>
    </div>

  <TMPL_IF list>

    <div class="section">

      If the presented information was useful, press "+", and if not press "-".<br />
      If you cannot decide, you do not need to press either. After the current batch is submitted new ones will be displayed.<br />

      <div class="tweet list">
        <form action="webigator-run.cgi" method="post">
        <input type="hidden" name="task_id" value="<TMPL_VAR task_id>">
        <input type="hidden" name="pass" value="<TMPL_VAR pass>">
        <table width="100%">
          <tr>
            <th class="tl_td_label">Label</th>
            <th>Text</th>
            <th>Tweet ID</th>
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
        <input type="submit" name="post_labels" value="Submit Labels" class="btn" />
        </form>
      </div>
    </div>

  <TMPL_ELSE>

    <br />
    <p class="error">Currently there is no data to be displayed. Either there was no text to be found for this keyword, or there is no program running to find text.</p>

    <form action="webigator-run.cgi" method="post">
    <input type="hidden" name="task_id" value="<TMPL_VAR task_id>">
    <input type="hidden" name="pass" value="<TMPL_VAR pass>">
    <input type="submit" value="Reload" class="btn" />
    </form>

  </TMPL_IF>

</TMPL_IF>

    <!-- end .content --></div>
  <div class="footer">
    <!-- end .footer --></div>
  <!-- end .container --></div>
</body>
</html>
