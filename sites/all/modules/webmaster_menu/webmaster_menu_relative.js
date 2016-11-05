/*
Webmaster menu is positioned relatively

This script is included when either admin menu or toolbar is also present, 
in order to  set the top-margin according to height of admin menu (which depends
on the screen width, so it is dynamic)

*/


(function($) {


function setTopMargin(hasAdminMenu) {
  var height;
  if (hasAdminMenu) {
    height = $('#admin-menu').height();
  }
  else {
    height = $('#toolbar').height();
  }
  if (height) {
    $('.webmaster-menu').css('margin-top', height + 'px');
  }
}

function adminMenuReady() {
  $('#admin-menu').find('.shortcut-toggle').click(function () {
    setTopMargin(true);
  });
  setTopMargin(true);
}

Drupal.behaviors.webmasterMenu = {
  attach: function (context, settings) {

//    $('body').addClass('webmaster-menu-fixed');

    var hasAdminMenu = settings['webmaster_menu']['has_admin_menu'];
    if (hasAdminMenu) {
      var $adminMenu = $('#admin-menu');
      if ($adminMenu.length == 0) {
        // Admin menu is going to be fetched from cache
        // when this has been done, admin menu triggers a window.resize
        $(window).bind('resize.webmastermenu', adminMenuReady);
      }
      else {
        adminMenuReady();
      }
    }
    else {
      setTopMargin(false);
    }      


  }
};


})(jQuery);
