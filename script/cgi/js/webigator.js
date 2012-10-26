function toggleMemo(){
  $('.memo_text').slideToggle('fast', function(){
    $.cookie('webigator_memo_display', $(this).css('display'), {expires:30});
  });

  return false;
}

// onload
$(function(){
  if ($.cookie('webigator_memo_display') != 'none')
    $('.memo_text').show();
});