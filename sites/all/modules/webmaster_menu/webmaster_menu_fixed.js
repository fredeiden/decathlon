

(function($) {

function engulfToolbars() {
  $('body').addClass('webmaster-menu-engulfing');

  var $adminMenu = $('#admin-menu');

  var hasToolbar = ($('.toolbar-menu').length == 1);

  if (($adminMenu.length == 1) || (hasToolbar)) {
    $('body').append('<div id="webmaster_menu_engulfing_other_toolbars"></div>');

    if ($adminMenu.length == 1) {
      $adminMenu.appendTo("#webmaster_menu_engulfing_other_toolbars");
      $adminMenu.css('position', 'relative'); // override the css attribute set in admin_menu.js

      var $toggle = $adminMenu.find('.shortcut-toggle');
      $toggle.click(function () {
        setTopMargin();
      });
    }

    if (hasToolbar) {
      var $toolbar = $('#toolbar');
      $toolbar.appendTo("#webmaster_menu_engulfing_other_toolbars");
      $toolbar.css('position', 'relative');
    }

    $(".webmaster-menu").appendTo("#webmaster_menu_engulfing_other_toolbars");

    $(window).bind('resize.webmastermenu', function() {
      setTopMargin();
    });

    setTopMargin();
  }
}

function setTopMargin() {
  var height = $('#webmaster_menu_engulfing_other_toolbars').height();
  $('html').css('padding-top', height + 'px');
  // We don't set it on body element, because it is already used
  // by toolbar and admin menu. It is simpler to "undeclare" these in css

}


Drupal.behaviors.webmasterMenu = {
  attach: function (context, settings) {

    $('body').addClass('webmaster-menu-fixed');
    
    var hasAdminMenu = settings['webmaster_menu']['has_admin_menu'];
    if (hasAdminMenu) {
      var $adminMenu = $('#admin-menu');
      if ($adminMenu.length == 0) {
        // Admin menu is going to be fetched from cache
        // when this has been done, admin menu triggers a window.resize
        $(window).bind('resize.webmastermenu', function() {
          $(window).unbind('resize.webmastermenu');
          engulfToolbars();
        });
      }
      else {
        engulfToolbars();
      }
    }
    else {
      var hasToolbar = ($('.toolbar-menu').length == 1);
      if (hasToolbar) {
        engulfToolbars();
      }
    }

  }
};


})(jQuery);
