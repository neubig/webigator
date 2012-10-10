function toggleMemo(){
  $('.memo_text').slideToggle('fast', function(){
	  $.cookie('webigator_memo_display', $(this).css('display'));
	});

  return false;
}

function toggleAddKeyword() {
	$('.add_keyword').toggle();
}

// onload
$(function(){

	if ($.cookie('webigator_memo_display') != 'none')
  	toggleMemo();

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