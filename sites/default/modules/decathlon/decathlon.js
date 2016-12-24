(function ($) {
    Drupal.behaviors.decathlon = {
	attach: function (context, settings) {

	    $(".ordinal").text(function( i, n ) {
		var s = [ "th", "st", "nd", "rd" ],
		    v = n % 100;
		return n + ( s[ (v-20) % 10 ] || s[ v ] || s[ 0 ] );
		});

	    $(".player").each(function() {
		    $(this).parent("h3").addClass($(this).attr('class').split(' ')[1]);
		});

	}
    };
}(jQuery));
