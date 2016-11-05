(function(Drupal)
 {
     Drupal.ajax.prototype.commands.redirectUser = function(ajax, response)
     {
	 // response.path is the path we gave in our Ajax callback
	 window.location.reload(response.path);
     };
 }(Drupal));
