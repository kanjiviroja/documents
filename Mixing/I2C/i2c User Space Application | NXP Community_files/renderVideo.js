define([
    'jquery',
    'underscore',
    './defaults',
    './resizePlayer',
    'i18nReady!',
    'domReady!',
    'soy!jive.videos.i18n.keys.videoEncodingText'
], function ($, _, defaults, resizePlayer) {

    function shouldDetermineWidthByImage($div, width) {
        function isResponsive() {
            return $('body').hasClass('j-responsive-sm');
        }
        function elementHasPreviewImage() {
            return $div.find('.j-video-preview').length;
        }
        function elementIsInATile() {
            return $('.j-tile-body').length && _.filter($('.j-tile-body'), function($element) {return $element.contains($div[0])}).length;
        }
        function isWidthInvalid() {
            return !(width > 0);
        }
        return isWidthInvalid() && (($div.length && elementIsInATile()) || isResponsive() || elementHasPreviewImage());
    }

    function renderVideo(divID, video) {

        var $div = $('#' + divID);
        if (!video.ready) {
            var $warningDiv = $(
                '<div class="jive-video-encode-box">' +
                '<div class="jive-video-message-large">' + jive.videos.i18n.keys.videoEncodingText() + '</div>' +
                '<div class="jive-video-encode-icon"><span class="jive-icon-glyph icon-j-mov"></span><span class="jive-icon-glyph icon-gear"></span></div>' +
                '</div>');

            $div.replaceWith($warningDiv);
            return;
        }

        var externalVideoID = video.externalID ? '' + video.externalID : '';
        var player = resizePlayer($div);
        player.width = shouldDetermineWidthByImage($div, player.width) ? $div.find('img').width() : player.width;
        var width = player.width || Math.min(video.width, defaults.width);
        if ($('#j-stream, #j-js-communications').length === 0) {
            width = Math.min(width, $('.jive-video-base').width() - 10) > 0
                ? Math.min(width, $('.jive-video-base').width() - 10)
                : width;
        }

        var height = player.height || Math.min(video.height, Math.round(width / defaults.aspectRatio));

        if ($div.closest('#j-stream, #j-js-communications').length > 0) {
            width = Math.min($div.find('img').width(), width);
            height = Math.min($div.find('img').height(), width);
        }

        $('.jive-video-content').height(height + 50);

        // Directly embedding this won't work because header_javascript.ftl blocks document.write calls
        // so use iFrame to host the video and still OVP code
        var $videoIframe = $('<iframe'
            + ' width="' + (width + 25) + '"'
            + ' height="' + (height + 25) + '"'
            + ' allowfullscreen="true"'
            + ' class="j-rte-video"'
            + ' id="iframe_' + externalVideoID + '"'
            + ' src="/standalone-video-player.jspa?'
            + 'videoId=' + externalVideoID + '&'
            + 'width=' + width + '&'
            + 'height=' + height + '&'
            + 'authtoken=' + video.authtoken + '&'
            + 'inline=' + video.inline
            + '">');

        // Swap out video and placeholder image
        $div.replaceWith($videoIframe);

        // what does this do?
        if ($videoIframe[0].contentWindow == null) {
            $div.replaceWith($videoIframe);
        }

    }

    return renderVideo;
});
