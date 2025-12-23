#pragma once

#include <vector>

#include "iara\Renderer\Texture.h"
#include "iara\Renderer\Framebuffer.h"
#include "iara\Renderer\shader.h"
#include "iara\Renderer\UniformBuffer.h"
#include "iara\Renderer\VertexArray.h"
#include <glm/gtc/type_ptr.hpp>



/**
 * Copyright (c) 2017 Eric Bruneton
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*<h2>atmosphere/constants.h</h2>

 <p>This file defines the size of the precomputed texures used in our atmosphere
 model. It also provides tabulated values of the <a href=
 "https://en.wikipedia.org/wiki/CIE_1931_color_space#Color_matching_functions"
 >CIE color matching functions</a> and the conversion matrix from the <a href=
 "https://en.wikipedia.org/wiki/CIE_1931_color_space">XYZ</a> to the
 <a href="https://en.wikipedia.org/wiki/SRGB">sRGB</a> color spaces (which are
 needed to convert the spectral radiance samples computed by our algorithm to
 sRGB luminance values).
 */





namespace iara {
		

		

		

		class DensityProfileLayer {
		public:
			DensityProfileLayer() : DensityProfileLayer(0.0, 0.0, 0.0, 0.0, 0.0) {}
			DensityProfileLayer(double width, double exp_term, double exp_scale,
				double linear_term, double constant_term)
				: width(width), exp_term(exp_term), exp_scale(exp_scale),
				linear_term(linear_term), constant_term(constant_term) {
			}
			double width;
			double exp_term;
			double exp_scale;
			double linear_term;
			double constant_term;
		};

		class Atmosphere {
		public:
			Atmosphere(const std::vector<double>& wavelenghts, const std::vector<double>& solar_irradiance,
				double sun_angular_radius, double bottom_radius, double top_radius,
				const std::vector<DensityProfileLayer>& rayleigh_density, const std::vector<double>& rayleigh_scattering,
				const std::vector<DensityProfileLayer>& mie_density, const std::vector<double>& mie_scattering, const std::vector<double>& mie_extinction,
				double mie_phase_function_g,
				const std::vector<DensityProfileLayer>& absorbtion_density, const std::vector<double>& absorbtion_extinction,
				const std::vector<double>& ground_albedo,
				double max_sun_zenith_angle, double length_unit_in_meters, uint32_t num_precomputed_wavelengths);

			~Atmosphere();

			void init(uint32_t num_scattering_orders = 4);

			inline Ref<Texture2D> getTransmittanceTex() { return m_transmittance_tex; }
			inline Ref<Texture2D> getIrradianceTex() { return m_irradiance_tex; }
			inline Ref<Texture3D> getScatteringTex() { return m_scattering_tex; }
			inline Ref<Texture3D> getMieScatteringTex() { return m_single_mie_scattering_tex; }

			static void ConvertSpectrumToLinearSrgb(
				const std::vector<double>& wavelengths,
				const std::vector<double>& spectrum,
				double* r, double* g, double* b);

			
		private:
			void precompute(uint32_t fbo, Ref<Texture2D> delta_irradiance, Ref<Texture3D> delta_rayleigh_scattering,
							Ref<Texture3D> delta_mie_scattering, Ref<Texture3D> delta_scattering_density, Ref<Texture3D> delta_multiple_scattering,
							const glm::vec3& lambdas, const glm::mat4& luminance_from_radiance, bool blend, uint32_t num_scattering_orders);

		private:
			uint32_t m_num_precomputed_wavelengts;

			Ref<Texture2D> m_transmittance_tex = nullptr;
			Ref<Texture3D> m_scattering_tex = nullptr;
			Ref<Texture3D> m_single_mie_scattering_tex = nullptr;
			Ref<Texture2D> m_irradiance_tex = nullptr;
			Ref<VertexArray> m_quad_vao = nullptr;
			Ref<VertexBuffer> m_quad_vb = nullptr;
			Ref<Shader> m_atmosphere_shader = nullptr;

			Ref<Shader> compute_transmittance       = nullptr;
			Ref<Shader> compute_direct_irradiance   = nullptr;
			Ref<Shader> compute_single_scattering   = nullptr;
			Ref<Shader> compute_scattering_density  = nullptr;
			Ref<Shader> compute_indirect_irradiance = nullptr;
			Ref<Shader> compute_multiple_scattering = nullptr;

			Ref<UniformBuffer> luminance_matrix_ubo = nullptr;
			Ref<UniformBuffer> layer_ubo = nullptr;
			Ref<UniformBuffer> scattering_order_ubo = nullptr;
		};


}