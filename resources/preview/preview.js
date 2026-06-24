(function () {
  function setupBridge() {
    if (!window.QWebChannel) return;
    new QWebChannel(qt.webChannelTransport, function (channel) {
      window.slateBridge = channel.objects.bridge;
    });
  }
  function installCopyButtons() {
    document.querySelectorAll(".copy-code").forEach(function (button) {
      button.addEventListener("click", function () {
        var code = button.closest(".code-block").querySelector("code").innerText;
        navigator.clipboard.writeText(code).then(function () {
          button.innerText = "Copied";
          setTimeout(function () { button.innerText = "Copy"; }, 1200);
        });
      });
    });
  }
  function publishScroll() {
    if (!window.slateBridge) return;
    var max = document.documentElement.scrollHeight - window.innerHeight;
    window.slateBridge.previewScrolled(max <= 0 ? 0 : window.scrollY / max);
  }
  window.__slateMarkScrollToRatio = function (ratio) {
    var max = document.documentElement.scrollHeight - window.innerHeight;
    window.scrollTo({ top: max * ratio, behavior: "auto" });
  };
  document.addEventListener("click", function (event) {
    var heading = event.target.closest("[data-source-line]");
    if (heading && window.slateBridge) {
      window.slateBridge.previewClicked(parseInt(heading.dataset.sourceLine, 10));
    }
  });
  window.addEventListener("scroll", publishScroll, { passive: true });
  setupBridge();
  installCopyButtons();
})();

