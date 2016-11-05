(function ($) {
    Drupal.behaviors.decathlon_scoring = {
	attach: function (context, settings) {

	    // on click, make group active
	    $(".scoring-group .player-image").click(function() {
		    $(".scoring-group .player-image.active").removeClass("active");
		    $(this).addClass("active");
		    $(this).children(".invalid").remove();
		});

	    // on click, clear status
	    $(".layout .form-item").click(function() {
		    $(".player-names .views-row").each(function() {
			    $(this).css("background", "#eee");
			    $(this).data("status", "");
			});
		});

	    // on click, add player to group
	    $(".player-names .views-row").click(function() {

		    // prevent same player twice
		    var status = $(this).data("status");
		    if (status == "selected") {
			return false;
		    }

		    // pick player to add
		    var player = $(".player-image.active");
		    if (player.length === 0) { // user did not select a box, find the next available
			$(".scoring-group .player-image").each(function() {
				if (!$(this).find(".field-content").length) {
				    player = $(this);
				    return false;
				}
			    });
		    }
		    if (player.length === 0) { // no available boxes
			return false;
		    }

		    // reset highlighted row
		    var row = player.data("row");
		    var that = $(".player-names .views-row." + row);
		    that.css("background", "#eee");
		    that.data("status", "");

		    // highlight new row
		    $(this).css("background", "#fff");
		    $(this).data("status", "selected");
		    // reset data
		    player.data("row", $(this).attr("class").split(" ")[1]);

		    // remove current image and any warning
		    $(".field-content", player).remove();

		    // add new image
		    player.append($(this).find(".views-field-field-picture .field-content").clone());

		    // set hidden form value
		    var id = (player.parent()).next();
		    id.val($(this).find(".views-field-tid .field-content").text());

		    // remove active class
		    player.removeClass("active");
		});

	    // ordinal names
	    $(".ordinal").text(function( i, n ) {
		    var s = [ "th", "st", "nd", "rd" ],
			v = n % 100;
		    return n + ( s[ (v-20) % 10 ] || s[ v ] || s[ 0 ] ) + " Place";
		});

	    // better radios
	    $(".buttonset").buttonset();

	    // scrollbars
	    $(".newscroll").mCustomScrollbar({theme:"inset-dark"});

	    // validation on for submit: make sure all player fields populated
	    $("#edit-submit").click(function(event) {
		    var pass = true;
		    $(".scoring-group .player-image").each(function() {
			    var img = $(this).children(".field-content");
			    if (img.length === 0) {
				$(this).append("<span class='invalid'>Add Player</span>");
				pass = false;
			    }
			});
		    return pass;
		});

	    // skip validation for number input when form cancelled
	    $("#edit-cancel").click(function(event) {
		    $("input[type='number']").removeAttr("required");
		});
	}
    };
}(jQuery));
