// https://learn.microsoft.com/en-us/windows-server/security/windows-authentication/security-support-provider-interface-architecture
export module boring32:sspi;
export import :sspi_credential;
export import :sspi_securitycontext;
export import :sspi_types;
export import :sspi_contextbuffer;
export import :sspi_sizedcontextbuffer;
export import :sspi_buffertype;
export import :sspi_provider_credential;
export import :sspi_provider_digest;
export import :sspi_provider_negotiate;
export import :sspi_provider_ntlm;
export import :sspi_provider_kerberos;
export import :sspi_provider_pku2u;
export import :sspi_provider_schannel;
