---
layout: default
title: Home
nav_order: 1
description: "SHAPEIT5 is a tool for haplotype phasing of high coverage sequencing data."
permalink: /
---
{: .warning }
Website under construction.<br>
A full release of the software, docker images, tutorials and documentation is scheduled for <b>Tuesday the 15th of November 2022</b>.<br>
Please contact olivier.delaneau@unil.ch if you have questions.



<!---
<img src="assets/images/branding/shapeit_logo.png" align="right" alt="Shapeit5" style="height:150px">
-->

# SHAPEIT5
{: .fs-9 .fw-500 }

**S**egmented **HAP**lotype **E**stimation and **I**mputation **T**ools version **5**
{: .fs-5 }

## About

SHAPEIT5 estimates haplotypes in large datasets, with a special focus on rare variants. 

## Citation

If you use SHAPEIT5 in your research work, please cite the following paper:

[Accurate rare variant phasing of whole-genome and whole-exome sequencing data in the UK Biobank. <br>BioRxiv (2022) doi: https://doi.org/10.1101/2022.10.19.512867 ](https://www.biorxiv.org/content/10.1101/2022.10.19.512867v1)

---

[Get started now](#getting-started){: .btn .btn-primary .fs-5 .mb-4 .mb-md-0 .mr-2 .mx-auto }
[View source code on GitHub](https://github.com/odelaneau/shapeit5){: .btn .fs-5 .mb-4 .mb-md-0 }


## Description

SHAPEIT5 is composed of the following tools:

- **phase_common**. Tool to phase common sites, typically SNP array data, or the first step of WES/WGS data.
- **ligate**. Ligate multiple phased BCF/VCF files into a single whole chromosome file. Typically run to ligate multiple chunks of phased common variants.
- **phase_rare**. Tool to phase rare variants onto a scaffold of common variants (output of phase_common / ligate).
- **switch**. Program to compute switch error rate and genotyping error rate given simulated or trio data.

[phase_common]({{site.baseurl}}{% link docs/documentation/phase_common.md %}){: .btn .btn-blue }
[ligate]({{site.baseurl}}{% link docs/documentation/ligate.md %}){: .btn .btn-blue }
[phase_rare]({{site.baseurl}}{% link docs/documentation/phase_rare.md %}){: .btn .btn-blue }
[switch]({{site.baseurl}}{% link docs/documentation/switch.md %}){: .btn .btn-blue  }

## News

{: .new }
> **Version `1.0.0` will be soon available!**
> See [the CHANGELOG](https://github.com/odelaneau/shapeit5/blob/main/docs/CHANGELOG.md) for details.

---

## Getting started

- [See documentation]({{site.baseurl}}{% link docs/documentation/documentation.md %})

---

## About the project

SHAPEIT5 is developed by [Olivier Delaneau](https://odelaneau.github.io/lap-page).

### License

SHAPEIT5 is distributed with [MIT license](https://github.com/odelaneau/shapeit5/blob/main/LICENSE).

### Organisations

<div class="d-flex justify-content-around">
  <div class="p-5"><a href="https://www.unil.ch/index.html"><img src="assets/images/lausanne_logo.jpg" align="right" alt="unil" style="height:50px"></a></div>
  <div class="p-5"><a href="https://www.sib.swiss/"><img src="assets/images/sib_logo.jpg" align="right" alt="sib" style="height:50px"></a></div>
  <div class="p-5"><a href="https://www.snf.ch/en/Pages/default.aspx"><img src="assets/images/snf.gif" align="right" alt="snf" style="height:50px"></a></div>
</div>

### Contributing

SHAPEIT5 is an open source project and we very much welcome new contributors. When contributing to our repository, please first discuss the change you wish to make via issue,
email, or any other method with the owners of this repository before making a change.
#### Thank you to the contributors of SHAPEIT5!

<ul class="list-style-none">
{% for contributor in site.github.contributors %}
  <li class="d-inline-block mr-1">
     <a href="{{ contributor.html_url }}"><img src="{{ contributor.avatar_url }}" width="32" height="32" alt="{{ contributor.login }}"/></a>
  </li>
{% endfor %}
</ul>

We thank the [Just the Docs](https://github.com/just-the-docs/just-the-docs) developers, who made this awesome theme for Jekyll.
