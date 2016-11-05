(function ($) {
    Drupal.behaviors.decathlon_ticker = {
	attach: function (context, settings) {

	    $("#ticker").liScroll({travelocity: 0.07});


	}
    };
}(jQuery));
