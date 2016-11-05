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

	    $('.eaderboard-game-standings .item-list').scrollbox({
		    startDelay: 10,
		    linear: true,
			step: 1,
			delay: 0,
			speed: 40,
			onMouseOverPause: true,
			});

	    $('.ane-decathlon-leaderboard .item-list').scrollbox({
		    startDelay: 10,
		    onMouseOverPause: true,
		    speed: 30,
		    delay: 0,
		    step: 1,
		    linear: true,
			});

	}
    };
}(jQuery));
