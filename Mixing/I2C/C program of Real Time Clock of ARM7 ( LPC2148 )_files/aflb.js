jQuery(document).ready(function() {
    var placement = {};
    if("click"==aflb.slideon.replace(/"/g, "") || ((navigator.userAgent.match(/iPhone/i)) || (navigator.userAgent.match(/iPod/i)) || (navigator.userAgent.match(/iPad/i)))) {
        jQuery(".aspexifblikebox").click(function() {
            if(0==parseInt(jQuery(this).css(aflb.placement),10)) {
                placement[aflb.placement] = -(parseInt(aflb.width)+5);
                jQuery(this).stop().animate(placement, 400);
            } else {
                placement[aflb.placement] = 0;
                jQuery(this).stop().animate(placement, 400);
            }
        });
    } else {
        jQuery(".aspexifblikebox").hover(function() {
            placement[aflb.placement] = 0;
            jQuery(this).stop().animate(placement, 400);
        }, function() {
            placement[aflb.placement] = -(parseInt(aflb.width)+5);
            jQuery(this).stop().animate(placement, 400);
        });
    }

});