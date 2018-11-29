// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/utility/atom_content_utility_client.h"

#include <string>
#include <utility>

#include "base/command_line.h"
#include "content/public/child/child_thread.h"
#include "services/proxy_resolver/proxy_resolver_service.h"
#include "services/proxy_resolver/public/mojom/proxy_resolver.mojom.h"
#include "services/service_manager/sandbox/switches.h"

#if BUILDFLAG(ENABLE_PRINTING)
#include "chrome/services/printing/printing_service.h"
#include "chrome/services/printing/public/mojom/constants.mojom.h"
#include "components/services/pdf_compositor/public/cpp/pdf_compositor_service_factory.h"
#include "components/services/pdf_compositor/public/interfaces/pdf_compositor.mojom.h"

#if defined(OS_WIN)
#include "chrome/utility/printing_handler_win.h"
#endif  // defined(OS_WIN)

#endif  // BUILDFLAG(ENABLE_PRINTING)

namespace atom {

AtomContentUtilityClient::AtomContentUtilityClient() {
#if defined(OS_WIN)
  handlers_.push_back(std::make_unique<printing::PrintingHandlerWin>());
#endif
}

AtomContentUtilityClient::~AtomContentUtilityClient() {}

bool AtomContentUtilityClient::OnMessageReceived(const IPC::Message& message) {
#if defined(OS_WIN)
  for (const auto& handler : handlers_) {
    if (handler->OnMessageReceived(message))
      return true;
  }
#endif

  return false;
}

void AtomContentUtilityClient::RegisterServices(StaticServiceMap* services) {
  service_manager::EmbeddedServiceInfo proxy_resolver_info;
  proxy_resolver_info.task_runner =
      content::ChildThread::Get()->GetIOTaskRunner();
  proxy_resolver_info.factory =
      base::BindRepeating(&proxy_resolver::ProxyResolverService::CreateService);
  services->emplace(proxy_resolver::mojom::kProxyResolverServiceName,
                    proxy_resolver_info);

#if BUILDFLAG(ENABLE_PRINTING)
  service_manager::EmbeddedServiceInfo pdf_compositor_info;
  pdf_compositor_info.factory =
      base::BindRepeating(&printing::CreatePdfCompositorService, std::string());
  services->emplace(printing::mojom::kServiceName, pdf_compositor_info);

  service_manager::EmbeddedServiceInfo printing_info;
  printing_info.factory =
      base::BindRepeating(&printing::PrintingService::CreateService);
  services->emplace(printing::mojom::kChromePrintingServiceName, printing_info);
#endif
}

}  // namespace atom
