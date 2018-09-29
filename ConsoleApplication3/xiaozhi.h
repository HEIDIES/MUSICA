/**
 * @defgroup enumerations Public enumeration types
 */

#ifndef XIAOZHI_API_XIAOZHI_H_
#define XIAOZHI_API_XIAOZHI_H_
#pragma once

#define STRINGIFY_HELPER(X) #X
#define STRINGIFY(X) STRINGIFY_HELPER(X)

#ifdef _WIN32
#define DECL_EXPORT __declspec(dllexport)
#define DECL_IMPORT __declspec(dllimport)
#define DECL_HIDDEN
#else
#define DECL_EXPORT __attribute__((visibility("default")))
#define DECL_IMPORT __attribute__((visibility("default")))
#define DECL_HIDDEN __attribute__((visibility("hidden")))
#endif

#ifdef DOXYGEN_WORKING

#define XIAOZHI_API

#else

#ifdef XIAOZHI_EXPORTS
#define XIAOZHI_API DECL_EXPORT
#else
#define XIAOZHI_API DECL_IMPORT
#endif

#endif

#if defined(CXX11) || defined(CXX0X)
#define ENUM(name) enum class name : std::int8_t
#else
#define ENUM(name) enum name
#endif

#define MAX_WORD 65535
#define MAX_BYTE 255

namespace myMusica {

	/**
	 * @ingroup enumerations
	 * @brief List error codes.
	 */
	ENUM(ErrorCode) {
		/** Standard code for successful behavior. */
		SUCCESS = 0,
		/** Standard code for unsuccessful behavior. */
		ERROR_FAILURE,
		/** File cannot be opened for not exist, not a regular file or any other reason. */
		ERROR_FILE_OPEN_FAILED,
		/** Memeroy copy failed */
		ERROR_MEMERORY_COPY_FAILED

	};

	ENUM(RotateDirection) {
		/** Rotate clockwise by 90 degree. */
		CLOCKWISE = 0,
		/** Rotate counterclockwise by 90 degree. */
		COUNTERCLOCKWISE
	};

} // namespace myMusica


#endif