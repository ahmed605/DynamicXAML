uint16_t DynamicLoader::RegisterXamlMetadataProvider(::winrt::XAML_NAMESPACE::Markup::IXamlMetadataProvider const& provider)
{
    std::lock_guard<std::mutex> lock(DynamicXaml_MetadataProvider::implementation::DynamicXamlMetadataProvider::s_mutex);

	uint16_t token = DynamicXaml_MetadataProvider::implementation::DynamicXamlMetadataProvider::s_nextToken++;
    DynamicXaml_MetadataProvider::implementation::DynamicXamlMetadataProvider::s_providers[token] = provider;
	return token;
}

void DynamicLoader::UnregisterXamlMetadataProvider(uint16_t token)
{
	std::lock_guard<std::mutex> lock(DynamicXaml_MetadataProvider::implementation::DynamicXamlMetadataProvider::s_mutex);
	DynamicXaml_MetadataProvider::implementation::DynamicXamlMetadataProvider::s_providers.erase(token);
}